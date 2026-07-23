#pragma once

#include <utility>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class TimelinePatternRenderNode final : public RenderTree
  {
  public:
    explicit TimelinePatternRenderNode(std::vector<RenderTreePtr> children)
    : mChildren(std::move(children))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end || mChildren.empty())
      {
        return {};
      }

      const auto numerator = request.start.getNumerator();
      const auto denominator = request.start.getDenominator();
      auto cycleIndex = numerator / denominator;
      if (((numerator < 0) != (denominator < 0)) && ((numerator % denominator) != 0))
      {
        cycleIndex -= 1;
      }
      const auto count = static_cast<long>(mChildren.size());
      const auto childIndex = ((cycleIndex % count) + count) % count;
      const auto& pChild = mChildren[static_cast<size_t>(childIndex)];
      if (!pChild)
      {
        return {};
      }

      QueryRequest localRequest;
      localRequest.start = request.start - Fraction(cycleIndex);
      localRequest.end = request.end - Fraction(cycleIndex);

      const auto childFragments = pChild->query(localRequest);
      std::vector<QueryFragment> fragments;
      fragments.reserve(childFragments.size());
      for (const auto& childFragment : childFragments)
      {
        QueryFragment mapped;
        mapped.wholeStart = childFragment.wholeStart + Fraction(cycleIndex);
        mapped.wholeEnd = childFragment.wholeEnd + Fraction(cycleIndex);
        mapped.partStart = childFragment.partStart + Fraction(cycleIndex);
        mapped.partEnd = childFragment.partEnd + Fraction(cycleIndex);
        mapped.value = childFragment.value;
        fragments.push_back(mapped);
      }

      return fragments;
    }

  private:
    std::vector<RenderTreePtr> mChildren;
  };
}
