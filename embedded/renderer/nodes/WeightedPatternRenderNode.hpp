#pragma once

#include "RenderNode.hpp"
#include "utils/Weighting.hpp"

namespace krill
{
class WeightedPatternRenderNode : public RenderNode
{
public:
  explicit WeightedPatternRenderNode(RenderNodeArray& children)
    : mChildren(children)
  {}

  void tick() override
  {
    for (auto& child : mChildren)
    {
      child->tick();
    }
  }

  Cycle render() override
  {
    const auto events = detail::computeEventsFromWeightedArray(mChildren);
    return {1, events};
  }

  size_t stepCount() final
  {
    return mChildren.size();
  }

private:
  RenderNodeArray mChildren;
};

static RenderNodePtr makeWeightedPatternRenderNode(std::vector<RenderNodePtr>& children)
{
  return std::make_shared<WeightedPatternRenderNode>(children);
}
} // namespace krill
