#pragma once

#include "RenderNode.hpp"
#include "utils/Weaving.hpp"

#include <algorithm>

namespace krill
{
class ShiftRenderNode : public RenderNode
{
public:
  ShiftRenderNode(RenderNodePtr child, Fraction shift)
    : mpChild(child)
    , mShift(shift)
    , mUseDynamicShift(false)
  {}

  ShiftRenderNode(RenderNodePtr child, RenderNodePtr shiftArg, Fraction direction)
    : mpChild(child)
    , mpShiftArg(shiftArg)
    , mDirection(direction)
    , mUseDynamicShift(true)
  {}

  void tick() override
  {
    mpChild->tick();
    if (mUseDynamicShift)
    {
      mpShiftArg->tick();
    }
  }

  Cycle render() override
  {
    Cycle result = mpChild->render();

    if (mUseDynamicShift)
    {
      const auto shifts = mpShiftArg->render();
      if (result.length == Fraction(0))
      {
        return result;
      }

      EventArray shiftedEvents;
      shiftedEvents.reserve(result.events.size());

      for (const auto& e : result.events)
      {
        const auto shiftValues = detail::sampleCycle(shifts, e.time);
        for (const auto& shiftValue : shiftValues)
        {
          Fraction delta = detail::fractionFromValueOrZero(shiftValue);
          delta *= mDirection;

          auto shiftedTime = e.time + delta;
          while (shiftedTime >= result.length) shiftedTime -= result.length;
          while (shiftedTime < Fraction(0)) shiftedTime += result.length;
          shiftedTime.reduce();

          shiftedEvents.push_back(Cycle::Event{shiftedTime, e.values});
        }
      }

      return {result.length, detail::mergeEventsByTime(shiftedEvents)};
    }

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
  RenderNodePtr mpShiftArg;
  Fraction      mShift;
  Fraction      mDirection{1};
  bool          mUseDynamicShift;
};

static RenderNodePtr makeShiftRenderNode(RenderNodePtr child, Fraction shift)
{
  return std::make_shared<ShiftRenderNode>(child, shift);
}

static RenderNodePtr makeShiftRenderNode(RenderNodePtr child,
                                         RenderNodePtr shiftArg,
                                         Fraction direction)
{
  return std::make_shared<ShiftRenderNode>(child, shiftArg, direction);
}
} // namespace krill
