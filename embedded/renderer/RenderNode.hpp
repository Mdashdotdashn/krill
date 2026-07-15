#pragma once

#include "cycle/Cycle.hpp"
#include "utils/jsonUtils.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <memory>
#include <numeric>
#include <vector>

namespace krill
{

//------------------------------------------------------------------------------

class RenderNode
{
public:
  virtual void tick() = 0;
  virtual Cycle render() = 0;
  virtual size_t stepCount() {
    return 1;
  }

  void setWeight(float weight) { mWeight = weight; }
  float weigth() { return mWeight; }
private:
  float mWeight{ 1 };
};

using RenderNodePtr = std::shared_ptr<RenderNode>;
using RenderNodeArray = std::vector<RenderNodePtr>;

namespace detail
{
  static EventArray computeEventsFromWeightedArray(const RenderNodeArray& renderNodes)
  {
    const float totalWeight = std::accumulate(renderNodes.begin(), renderNodes.end(), 0.f, [](float acc, const RenderNodePtr& pRenderNode) { return acc + pRenderNode->weigth(); });
    Fraction weightFactor;
    weightFactor.convertDoubleToFraction(totalWeight);

    EventArray events;
    auto position = Fraction(0);

    for (const auto& pNode : renderNodes)
    {
      const auto cycle = pNode->render();
      assert(cycle.length == Fraction(1));
      const auto scaleFactor = Fraction(pNode->weigth()) / weightFactor;
      for (const auto& event : cycle.events)
      {
        auto scaled = Cycle::Event{ position + (event.time * scaleFactor), event.values };
        scaled.time.reduce();
        events.push_back(scaled);
      }
      position += scaleFactor;
    }

    return events;
  }
}

//------------------------------------------------------------------------------
// CycleRenderNode:
// Wraps a cycle as a render node

class CycleRenderNode: public RenderNode
{
public:
  CycleRenderNode(const Cycle& cycle)
  : mCycle(cycle)
  {}

  void tick() override
  {}

  Cycle render() override
  {
    return mCycle;
  }

  size_t stepCount() override
  {
    return mCycle.events.size();
  }

private:
  Cycle mCycle;
};

//------------------------------------------------------------------------------
// SliceRenderNode:
// Slices the cycles created by its mChild node in chunks a specified by
// the slice size.

class SliceRenderNode: public RenderNode
{
public:
  SliceRenderNode(RenderNodePtr pChild)
  : mpChild(pChild)
  {
  }

  void setSliceSize(const Fraction& size)
  {
    mSliceLength = size;
  }

  void tick() override
  {
    // Accumulate copies of the cycle until we have enough
    // to cover the slice length
    while (mAccumulator.length < mSliceLength)
    {
      mpChild->tick();
      Cycle newCycle = mpChild->render();
      mAccumulator = concat(mAccumulator, newCycle);
    }
  }

  Cycle render() override
  {
    const auto sliced = slice(mAccumulator, 0, mSliceLength);
    const auto remaining = mAccumulator.length - mSliceLength;
    if (remaining > Fraction(0))
    {
      mAccumulator = slice(mAccumulator, mSliceLength, remaining);
    }
    else
    {
      mAccumulator = Cycle{};
    }
    return sliced;
  }

private:
  RenderNodePtr mpChild{};
  Cycle mAccumulator{};
  Fraction mSliceLength{1};
};

static RenderNodePtr makeCycleRenderNode(const Cycle& cycle)
{
  return std::make_shared<CycleRenderNode>(cycle);
}


//------------------------------------------------------------------------------
// WeightedPatternRenderNode:
// resolves a serie of steps with applied weighting

class WeightedPatternRenderNode : public RenderNode
{
public:
  WeightedPatternRenderNode(RenderNodeArray& children)
    : mChildren(children)
  {}

  void tick()
  {
    for (auto& child : mChildren)
    {
      child->tick();
    }
  };

  Cycle render() 
  {
    const auto events = detail::computeEventsFromWeightedArray(mChildren);
    return {1, events};
  }

  size_t stepCount() final
  {
    return mChildren.size();
  }

private:
  RenderNodeArray mChildren;
};

static RenderNodePtr makeWeightedPatternRenderNode(std::vector<RenderNodePtr>& children)
{
  return std::make_shared<WeightedPatternRenderNode>(children);
}

//------------------------------------------------------------------------------
// StretchRenderNode:
// Stretches/Compresses the content of the underlying source cycle

class StretchRenderNode : public RenderNode
{
public:
  StretchRenderNode(RenderNodePtr& child, Fraction stretchFactor)
    : mpChild(child)
    , mStretchFactor(stretchFactor)
  {}

  void tick()
  {
    mpChild->tick();
  }

  Cycle render()
  {
    Cycle result = mpChild->render();
    result.length *= mStretchFactor;
    for (auto& e : result.events)
    {
      e.time *= mStretchFactor;
    }
    return result;
  }
private:
  RenderNodePtr mpChild;
  Fraction mStretchFactor;
};

static RenderNodePtr makeStretchRenderNode(RenderNodePtr child, Fraction stretchFactor)
{
  return std::make_shared<StretchRenderNode>(child, stretchFactor);
}

static RenderNodePtr makeFixedStepRenderNode(RenderNodePtr child, Fraction stepDivision)
{
  const auto stretchFactor = Fraction(double(child->stepCount())) / Fraction(1) / stepDivision;
  return std::make_shared<StretchRenderNode>(child, stretchFactor);
}

//------------------------------------------------------------------------------
// ShiftRenderNode:
// Shifts all events in the cycle by a fixed time offset (rotR / rotL).
// Events that shift past the end of the cycle wrap around to the beginning.

class ShiftRenderNode : public RenderNode
{
public:
  ShiftRenderNode(RenderNodePtr child, Fraction shift)
    : mpChild(child)
    , mShift(shift)
  {}

  void tick() override { mpChild->tick(); }

  Cycle render() override
  {
    Cycle result = mpChild->render();
    const auto length = result.length;

    for (auto& e : result.events)
    {
      e.time = e.time + mShift;
      // Wrap into [0, length)
      while (e.time >= length) e.time = e.time - length;
      while (e.time < Fraction(0)) e.time = e.time + length;
      e.time.reduce();
    }

    // Re-sort after wrap-around may have changed order
    std::sort(result.events.begin(), result.events.end(),
              [](const Cycle::Event& a, const Cycle::Event& b) {
                return a.time < b.time;
              });

    return result;
  }

private:
  RenderNodePtr mpChild;
  Fraction      mShift;
};

static RenderNodePtr makeShiftRenderNode(RenderNodePtr child, Fraction shift)
{
  return std::make_shared<ShiftRenderNode>(child, shift);
}

//------------------------------------------------------------------------------
// StructRenderNode:
// Weaves two patterns: samples at right pattern's event times,
// applies a boolean filter: keeps left if right is truthy, else substitutes rest

namespace detail
{
  // Parse a value as boolean: true for 't', 'T', '1', 'true', etc.
  // false for 'f', 'F', '0', 'false', '~', etc.
  static bool boolValue(const std::string& s)
  {
    if (s.empty()) return false;
    const auto lower = [](char c) { return std::tolower(c); };
    const auto first = lower(s[0]);
    return first == 't' || first == '1';
  }

  // Sample a cycle at a specific time
  // Returns the values from the event at or before that time
  static std::vector<std::string> sampleCycle(const Cycle& cycle, Fraction time)
  {
    // Find the event at or before this time
    const Cycle::Event* lastEvent = nullptr;
    
    for (const auto& event : cycle.events)
    {
      if (event.time <= time)
      {
        lastEvent = &event;
      }
      else
      {
        break;
      }
    }
    
    if (lastEvent)
    {
      return lastEvent->values;
    }
    
    // No event at or before time, return rest
    return {"~"};
  }
}

class StructRenderNode : public RenderNode
{
public:
  StructRenderNode(RenderNodePtr left, RenderNodePtr right)
    : mpLeft(left)
    , mpRight(right)
  {}

  void tick() override
  {
    mpLeft->tick();
    mpRight->tick();
  }

  Cycle render() override
  {
    // Render both patterns
    const auto leftCycle = mpLeft->render();
    const auto rightCycle = mpRight->render();

    // Sample at right pattern's event times
    EventArray events;
    
    for (const auto& rightEvent : rightCycle.events)
    {
      // Sample left at this time
      const auto leftValues = detail::sampleCycle(leftCycle, rightEvent.time);
      
      // Apply struct operation: for each left value, 
      // keep it if right value is truthy, else use rest
      for (const auto& leftVal : leftValues)
      {
        for (const auto& rightVal : rightEvent.values)
        {
          const auto keep = detail::boolValue(rightVal);
          const auto result = keep ? leftVal : std::string("~");
          
          Cycle::Event event;
          event.time = rightEvent.time;
          event.values.push_back(result);
          events.push_back(event);
        }
      }
    }

    // Merge duplicate times
    std::map<Fraction, std::vector<std::string>> merged;
    for (const auto& event : events)
    {
      merged[event.time].insert(
        merged[event.time].end(),
        event.values.begin(),
        event.values.end()
      );
    }

    // Convert back to event array
    EventArray result;
    for (const auto& [time, values] : merged)
    {
      result.push_back(Cycle::Event(time, values));
    }

    return {Fraction(1), result};
  }

private:
  RenderNodePtr mpLeft;
  RenderNodePtr mpRight;
};

static RenderNodePtr makeStructRenderNode(RenderNodePtr left, RenderNodePtr right)
{
  return std::make_shared<StructRenderNode>(left, right);
}

//------------------------------------------------------------------------------
// TimelineRenderNode:
// Plays children cycles one after the other

class TimelineRenderNode : public RenderNode
{
public:
  TimelineRenderNode(RenderNodeArray& children)
    : mChildren(children)
    , mCurrent(mChildren.size() - 1)
  {}

  void tick()
  {
    mCurrent = (mCurrent + 1) % mChildren.size();
    mChildren[mCurrent]->tick();
  }

  Cycle render()
  {
    return mChildren[mCurrent]->render();
  }

private:
  RenderNodeArray mChildren;
  size_t mCurrent{0};
};

static RenderNodePtr makeTimelineRenderNode(RenderNodeArray children)
{
  return std::make_shared<TimelineRenderNode>(children);
}

//------------------------------------------------------------------------------
// StackRenderNode:
// Plays children cycles in parallel

class StackRenderNode : public RenderNode
{
public:
  StackRenderNode(RenderNodeArray& children)
    : mChildren(children)
  {}

  void tick()
  {
    for (auto& child : mChildren)
    {
      child->tick();
    }
  }

  Cycle render()
  {
    // At this point, cycles with different lenght might be problematic
    // one way to cope would be to wrap every child with a slice rendernode
    // See also the hardcoded length of 1 when returning the cycle
    EventArray events;
    for (auto& child : mChildren)
    {
      Cycle childCycle = child->render();
      for (auto& event : childCycle.events)
      {
        events.push_back(event);
      }
    }
    return { 1, mergeAndSort(events) };
  }

  size_t stepCount() final
  {
    return mChildren[0]->stepCount();
  }

private:
  RenderNodeArray mChildren;
};

static RenderNodePtr makeStackRenderNode(RenderNodeArray children)
{
  return std::make_shared<StackRenderNode>(children);
}
} // namespace krill
