#pragma once

#include "RenderNode.hpp"

namespace krill
{
class SliceRenderNode : public RenderNode
{
public:
  explicit SliceRenderNode(RenderNodePtr pChild)
    : mpChild(pChild)
  {}

  void setSliceSize(const Fraction& size)
  {
    mSliceLength = size;
  }

  void tick() override
  {
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
} // namespace krill
