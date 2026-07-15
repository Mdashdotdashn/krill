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

    return detail::weaveCycles(leftCycle,
                               rightCycle,
                               detail::WeaveSamplingMode::right,
                               [](const std::string& leftValue, const std::string& rightValue) {
                                 return detail::boolValue(rightValue)
                                          ? leftValue
                                          : std::string("~");
                               });
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
