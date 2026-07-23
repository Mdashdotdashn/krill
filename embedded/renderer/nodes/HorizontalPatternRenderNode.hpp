#pragma once

#include <utility>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class HorizontalPatternRenderNode final : public RenderTree
  {
  public:
    explicit HorizontalPatternRenderNode(std::vector<RenderTreePtr> children)
    : mChildren(std::move(children))
    {
    }

    explicit HorizontalPatternRenderNode(std::vector<std::string> values)
    : mValues(std::move(values))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end)
      {
        return {};
      }

      const bool hasChildren = !mChildren.empty();
      const auto count = hasChildren ? mChildren.size() : mValues.size();
      if (count == 0)
      {
        return {};
      }

      std::vector<QueryFragment> fragments;
      const auto countLong = static_cast<long>(count);
      const auto startNumerator = request.start.getNumerator();
      const auto startDenominator = request.start.getDenominator();
      auto startCycle = startNumerator / startDenominator;
      if (((startNumerator < 0) != (startDenominator < 0)) && ((startNumerator % startDenominator) != 0))
      {
        startCycle -= 1;
      }

      const auto endNumerator = request.end.getNumerator();
      const auto endDenominator = request.end.getDenominator();
      auto endCycle = endNumerator / endDenominator;
      if (((endNumerator < 0) != (endDenominator < 0)) && ((endNumerator % endDenominator) != 0))
      {
        endCycle -= 1;
      }

      for (long i = 0; i < countLong; i++)
      {
        for (long cycle = startCycle; cycle <= endCycle; cycle++)
        {
          const auto wholeStart = Fraction(cycle) + Fraction(i, countLong);
          const auto wholeEnd = Fraction(cycle) + Fraction(i + 1, countLong);
          if (request.end <= wholeStart || request.start >= wholeEnd)
          {
            continue;
          }

          const auto slotPartStart = request.start > wholeStart ? request.start : wholeStart;
          const auto slotPartEnd = request.end < wholeEnd ? request.end : wholeEnd;

          if (hasChildren)
          {
            const auto& pChild = mChildren[static_cast<size_t>(i)];
            if (!pChild)
            {
              continue;
            }

            const auto slotSize = wholeEnd - wholeStart;
            QueryRequest localRequest;
            localRequest.start = Fraction(cycle) + ((slotPartStart - wholeStart) / slotSize);
            localRequest.end = Fraction(cycle) + ((slotPartEnd - wholeStart) / slotSize);

            const auto childFragments = pChild->query(localRequest);
            for (const auto& childFragment : childFragments)
            {
              QueryFragment mapped;
              mapped.wholeStart = wholeStart + ((childFragment.wholeStart - Fraction(cycle)) * slotSize);
              mapped.wholeEnd = wholeStart + ((childFragment.wholeEnd - Fraction(cycle)) * slotSize);
              mapped.partStart = wholeStart + ((childFragment.partStart - Fraction(cycle)) * slotSize);
              mapped.partEnd = wholeStart + ((childFragment.partEnd - Fraction(cycle)) * slotSize);
              mapped.value = childFragment.value;
              fragments.push_back(mapped);
            }
            continue;
          }

          QueryFragment fragment;
          fragment.wholeStart = wholeStart;
          fragment.wholeEnd = wholeEnd;
          fragment.partStart = slotPartStart;
          fragment.partEnd = slotPartEnd;
          fragment.value = mValues[static_cast<size_t>(i)];
          fragments.push_back(fragment);
        }
      }

      return fragments;
    }

  private:
    std::vector<RenderTreePtr> mChildren;
    std::vector<std::string> mValues;
  };
}
