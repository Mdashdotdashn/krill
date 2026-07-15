#pragma once

#include "RenderNode.hpp"
#include "utils/Numeric.hpp"
#include "utils/Weaving.hpp"

namespace krill
{
class AddRenderNode : public RenderNode
{
public:
  AddRenderNode(RenderNodePtr left, RenderNodePtr right)
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
    const auto leftCycle = mpLeft->render();
    const auto rightCycle = mpRight->render();

    const auto eventTimes = detail::collectEventTimes(leftCycle, rightCycle);

    EventArray events;

    for (const auto& time : eventTimes)
    {
      const auto leftValues = detail::sampleCycle(leftCycle, time);
      const auto rightValues = detail::sampleCycle(rightCycle, time);

      for (const auto& leftVal : leftValues)
      {
        for (const auto& rightVal : rightValues)
        {
          const auto value = detail::addValues(leftVal, rightVal);

          Cycle::Event event;
          event.time = time;
          event.values.push_back(value);
          events.push_back(event);
        }
      }
    }

    return {Fraction(1), detail::mergeEventsByTime(events)};
  }

private:
  RenderNodePtr mpLeft;
  RenderNodePtr mpRight;
};

static RenderNodePtr makeAddRenderNode(RenderNodePtr left, RenderNodePtr right)
{
  return std::make_shared<AddRenderNode>(left, right);
}
} // namespace krill
