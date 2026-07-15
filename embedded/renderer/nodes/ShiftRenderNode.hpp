#pragma once

#include "RenderNodeBase.hpp"

#include <algorithm>

namespace krill
{
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
      while (e.time >= length) e.time = e.time - length;
      while (e.time < Fraction(0)) e.time = e.time + length;
      e.time.reduce();
    }

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
} // namespace krill
