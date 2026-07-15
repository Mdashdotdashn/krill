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

    EventArray result;

    for (const auto& rightEvent : rightCycle.events)
    {
      const auto leftValues = detail::sampleCycle(leftCycle, rightEvent.time);
      std::vector<std::string> values;
      for (const auto& leftVal : leftValues)
      {
        for (const auto& rightVal : rightEvent.values)
        {
          values.push_back(detail::boolValue(rightVal) ? leftVal : std::string("~"));
        }
      }
      result.push_back(Cycle::Event{rightEvent.time, std::move(values)});
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
