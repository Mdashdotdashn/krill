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

using Options = std::map<std::string, std::string>;

template <typename T>
T optionOrValue(const Options& v, const std::string& member, const T& defaultValue)
{
  const auto it = v.find(member);
  return it != v.end() ? T(atof(it->second.c_str())) : defaultValue;
}

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
      const auto scaleFactor = Fraction(pNode->weigth()) / weightFactor;
      for (const auto& event : cycle.events)
      {
        const auto scaled = Cycle::Event{ position + (event.time * scaleFactor), event.values };
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
  {}

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
      Cycle newCycle = mpChild->render();
      mAccumulator = concat(mAccumulator, newCycle);
      mpChild->tick();
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

static RenderNodePtr makeStepRenderNode(const Cycle& cycle, const Options& options)
{
  assert(options.size() == 0); // See buildPatternStep
  const auto pStepNode = std::make_shared<CycleRenderNode>(cycle);
  auto ptr = std::make_shared<SliceRenderNode>(pStepNode);
  const auto weight = optionOrValue<float>(options, "weigth", 1);
  ptr->setWeight(weight);
  return ptr;
}


//------------------------------------------------------------------------------
// SliceRenderNode:
// Slices the cycles created by its mChild node in chunks a specified by
// the slice size.

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
} // namespace krill
