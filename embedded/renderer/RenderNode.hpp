#pragma once

#include "cycle/Cycle.hpp"
#include "utils/jsonUtils.hpp"

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
} // namespace krill
