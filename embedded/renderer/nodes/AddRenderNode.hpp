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

    return detail::weaveCycles(leftCycle,
                               rightCycle,
                               detail::WeaveSamplingMode::both,
                               [](const std::string& leftValue, const std::string& rightValue) {
                                 return detail::addValues(leftValue, rightValue);
                               });
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
