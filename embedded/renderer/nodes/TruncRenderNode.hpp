#pragma once

#include "RenderNode.hpp"

namespace krill
{
class TruncRenderNode : public RenderNode
{
public:
  TruncRenderNode(RenderNodePtr child, Fraction length)
    : mpChild(child)
    , mLength(length)
  {}

  void tick() override
  {
    mpChild->tick();
  }

  Cycle render() override
  {
    Cycle result = mpChild->render();
    const Fraction truncatedLength = result.length * mLength;

    EventArray filtered;
    filtered.reserve(result.events.size());
    for (const auto& event : result.events)
    {
      if (event.time < truncatedLength)
      {
        filtered.push_back(event);
      }
    }

    result.length = truncatedLength;
    result.events = std::move(filtered);
    return result;
  }

private:
  RenderNodePtr mpChild;
  Fraction mLength;
};

static RenderNodePtr makeTruncRenderNode(RenderNodePtr child, Fraction length)
{
  return std::make_shared<TruncRenderNode>(child, length);
}
} // namespace krill