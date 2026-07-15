#pragma once

#include "RenderNodeBase.hpp"
#include "utils/Weaving.hpp"

#include <map>

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

    std::map<Fraction, std::vector<std::string>> merged;
    for (const auto& event : events)
    {
      merged[event.time].insert(merged[event.time].end(), event.values.begin(), event.values.end());
    }

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
} // namespace krill
