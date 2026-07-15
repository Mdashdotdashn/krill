#pragma once

#include "RenderNode.hpp"

namespace krill
{
class TimelineRenderNode : public RenderNode
{
public:
  explicit TimelineRenderNode(RenderNodeArray& children)
    : mChildren(children)
    , mCurrent(mChildren.size() - 1)
  {}

  void tick() override
  {
    mCurrent = (mCurrent + 1) % mChildren.size();
    mChildren[mCurrent]->tick();
  }

  Cycle render() override
  {
    return mChildren[mCurrent]->render();
  }

private:
  RenderNodeArray mChildren;
  size_t mCurrent{0};
};

static RenderNodePtr makeTimelineRenderNode(RenderNodeArray children)
{
  return std::make_shared<TimelineRenderNode>(children);
}
} // namespace krill
