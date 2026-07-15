#pragma once

#include "RenderNodeBase.hpp"

namespace krill
{
class CycleRenderNode : public RenderNode
{
public:
  explicit CycleRenderNode(const Cycle& cycle)
    : mCycle(cycle)
  {}

  void tick() override
  {}

  Cycle render() override
  {
    return mCycle;
  }

  size_t stepCount() override
  {
    return mCycle.events.size();
  }

private:
  Cycle mCycle;
};

static RenderNodePtr makeCycleRenderNode(const Cycle& cycle)
{
  return std::make_shared<CycleRenderNode>(cycle);
}
} // namespace krill
