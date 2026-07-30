#pragma once

#include <utility>

#include "renderer/RenderNode.hpp"

namespace krill
{
  class ElementRenderNode final : public RenderNode
  {
  public:
    explicit ElementRenderNode(std::string value)
    : mValue(std::move(value))
    {
    }

    explicit ElementRenderNode(RenderNodePtr sourceNode)
    : mpSourceNode(std::move(sourceNode))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end)
      {
        return {};
      }

      if (mpSourceNode)
      {
        return mpSourceNode->query(request);
      }

      const auto numerator = request.start.getNumerator();
      const auto denominator = request.start.getDenominator();
      auto cycleIndex = numerator / denominator;
      if (((numerator < 0) != (denominator < 0)) && ((numerator % denominator) != 0))
      {
        cycleIndex -= 1;
      }

      const auto localStart = request.start - Fraction(cycleIndex);
      const auto localEnd = request.end - Fraction(cycleIndex);

      const Fraction wholeStart(0);
      const Fraction wholeEnd(1);
      if (localEnd <= wholeStart || localStart >= wholeEnd)
      {
        return {};
      }

      QueryFragment fragment;
      fragment.wholeStart = wholeStart + Fraction(cycleIndex);
      fragment.wholeEnd = wholeEnd + Fraction(cycleIndex);
      fragment.partStart = (localStart > wholeStart ? localStart : wholeStart) + Fraction(cycleIndex);
      fragment.partEnd = (localEnd < wholeEnd ? localEnd : wholeEnd) + Fraction(cycleIndex);
      fragment.value = mValue;
      return {fragment};
    }

  private:
    std::string mValue;
    RenderNodePtr mpSourceNode{};
  };
}
