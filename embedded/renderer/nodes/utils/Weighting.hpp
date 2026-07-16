#pragma once

#include "../RenderNode.hpp"

#include <algorithm>
#include <cassert>
#include <numeric>

namespace krill
{
namespace detail
{
static EventArray computeEventsFromWeightedArray(const RenderNodeArray& renderNodes)
{
  const double totalWeight = std::accumulate(renderNodes.begin(),
                                             renderNodes.end(),
                                             0.0,
                                             [](double acc, const RenderNodePtr& pRenderNode) {
                                               return acc + pRenderNode->weight();
                                             });
  Fraction weightFactor;
  weightFactor.convertDoubleToFraction(totalWeight);

  EventArray events;
  auto position = Fraction(0);

  for (const auto& pNode : renderNodes)
  {
    const auto cycle = pNode->render();
    assert(cycle.length == Fraction(1));
    const auto scaleFactor = Fraction(pNode->weight()) / weightFactor;
    for (const auto& event : cycle.events)
    {
      auto scaled = Cycle::Event{ position + (event.time * scaleFactor), event.values };
      scaled.time.reduce();
      events.push_back(scaled);
    }
    position += scaleFactor;
  }

  return events;
}
} // namespace detail
} // namespace krill
