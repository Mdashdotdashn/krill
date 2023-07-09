#pragma once

#include "cycle/Cycle.hpp"

#include <rapidjson/document.h>

#include <memory>


namespace krill
{
namespace detail
{
  bool hasMember(const rapidjson::Value& v, const std::string& member)
  {
    const auto it = v.FindMember("member");
    return it != v.MemberEnd();
  }

  template <typename T>
  T optionOrValue(const rapidjson::Value& v, const std::string& member, const T& defaultValue)
  {
    return hasMember(v, member) ? float(v[member.c_str()].GetDouble()) : defaultValue;
  }

}

//------------------------------------------------------------------------------

class RenderNode
{
public:
  virtual void tick() = 0;
  virtual Cycle render() = 0;
};

using RenderNodePtr = std::shared_ptr<RenderNode>;

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
    const auto slice = subCycle(mAccumulator, 0, mSliceLength);
    const auto remaining = mAccumulator.length - mSliceLength;
    if (remaining > Fraction(0))
    {
      mAccumulator = subCycle(mAccumulator, mSliceLength, remaining);
    }
    else
    {
      mAccumulator = Cycle{};
    }
  }

private:
  RenderNodePtr mpChild{};
  Cycle mAccumulator{};
  Fraction mSliceLength{1};
};

//------------------------------------------------------------------------------

class WeigthedStepRenderNode : public RenderNode
{
public:
  WeigthedStepRenderNode(RenderNodePtr child, float weight)
  : mSlicer(child)
  , mWeight(weight)
  {}

  void tick() override
  {
    mSlicer.tick();
  }

  Cycle render() override
  {
    mSlicer.render();
  }

private:
  SliceRenderNode mSlicer;
  float mWeight;
};

RenderNodePtr makeStepRenderNode(const Cycle& cycle, const rapidjson::Value& options)
{
  assert(!detail::hasMember(options, "operator")); // See buildPatternStep
  const auto pStepNode = std::make_shared<CycleRenderNode>(cycle);
  const auto weigth = detail::optionOrValue<float>(options, "weigth", 1);
  return std::make_shared<WeigthedStepRenderNode>(pStepNode, weigth);
}
} // namespace krill
