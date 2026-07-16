#pragma once

#include "RenderNode.hpp"

namespace krill
{
class StackRenderNode : public RenderNode
{
public:
  explicit StackRenderNode(const RenderNodeArray& children)
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
    EventArray events;
    for (auto& child : mChildren)
    {
      Cycle childCycle = slice(child->render(), 0, 1);
      for (auto& event : childCycle.events)
      {
        events.push_back(event);
      }
    }
    return {1, mergeAndSort(events)};
  }

  size_t stepCount() final
  {
    return mChildren[0]->stepCount();
  }

private:
  RenderNodeArray mChildren;
};

static RenderNodePtr makeStackRenderNode(RenderNodeArray children)
{
  return std::make_shared<StackRenderNode>(children);
}
} // namespace krill
