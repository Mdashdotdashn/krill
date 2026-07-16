#pragma once

#include "RenderNode.hpp"

namespace krill
{
class StretchRenderNode : public RenderNode
{
public:
  StretchRenderNode(const RenderNodePtr& child, Fraction stretchFactor)
    : mpChild(child)
    , mStretchFactor(stretchFactor)
  {}

  void tick() override
  {
    mpChild->tick();
  }

  Cycle render() override
  {
    Cycle result = mpChild->render();
    result.length *= mStretchFactor;
    for (auto& e : result.events)
    {
      e.time *= mStretchFactor;
    }
    return result;
  }

private:
  RenderNodePtr mpChild;
  Fraction mStretchFactor;
};

static RenderNodePtr makeStretchRenderNode(RenderNodePtr child, Fraction stretchFactor)
{
  return std::make_shared<StretchRenderNode>(child, stretchFactor);
}

static RenderNodePtr makeFixedStepRenderNode(RenderNodePtr child, Fraction stepDivision)
{
  const auto stretchFactor = Fraction(double(child->stepCount())) / Fraction(1) / stepDivision;
  return std::make_shared<StretchRenderNode>(child, stretchFactor);
}
} // namespace krill
