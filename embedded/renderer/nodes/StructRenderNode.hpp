#pragma once

#include "RenderNode.hpp"
#include "utils/Weaving.hpp"

namespace krill
{
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
    const auto leftCycle = mpLeft->render();
    const auto rightCycle = mpRight->render();

    EventArray events;

    for (const auto& rightEvent : rightCycle.events)
    {
      const auto leftValues = detail::sampleCycle(leftCycle, rightEvent.time);

      for (const auto& leftVal : leftValues)
      {
        for (const auto& rightVal : rightEvent.values)
        {
          const auto keep = detail::boolValue(rightVal);
          const auto value = keep ? leftVal : std::string("~");

          Cycle::Event event;
          event.time = rightEvent.time;
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

static RenderNodePtr makeStructRenderNode(RenderNodePtr left, RenderNodePtr right)
{
  return std::make_shared<StructRenderNode>(left, right);
}
} // namespace krill
