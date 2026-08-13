#pragma once

#include <utility>

#include "../RenderNode.hpp"

namespace krill
{
  class HorizontalPatternRenderNode final : public RenderNode
  {
  public:
    HorizontalPatternRenderNode(std::vector<RenderNodePtr> children, std::vector<Fraction> weights)
    : mChildren(std::move(children)), mWeights(std::move(weights))
    {
      if (mWeights.size() != mChildren.size())
      {
        mWeights.assign(mChildren.size(), Fraction(1));
      }
      for (auto& weight : mWeights)
      {
        if (weight <= Fraction(0))
        {
          weight = Fraction(1);
        }
      }
    }

    explicit HorizontalPatternRenderNode(std::vector<RenderNodePtr> children)
    : HorizontalPatternRenderNode(std::move(children), std::vector<Fraction>{})
    {
      mWeights.assign(mChildren.size(), Fraction(1));
    }

    explicit HorizontalPatternRenderNode(std::vector<std::string> values)
    : mValues(std::move(values))
    {
      mWeights.assign(mValues.size(), Fraction(1));
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
      Fraction totalWeight(0);
      for (size_t i = 0; i < count; i++)
      {
        totalWeight += mWeights[i];
      }
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

      Fraction slotOffset(0);
      for (long i = 0; i < countLong; i++)
      {
        const auto slotWeight = mWeights[static_cast<size_t>(i)];
        const auto slotStartNormalized = slotOffset / totalWeight;
        slotOffset += slotWeight;
        const auto slotEndNormalized = slotOffset / totalWeight;

        for (long cycle = startCycle; cycle <= endCycle; cycle++)
        {
          const auto wholeStart = Fraction(cycle) + slotStartNormalized;
          const auto wholeEnd = Fraction(cycle) + slotEndNormalized;
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
              mapped.controls = childFragment.controls;
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
    std::vector<RenderNodePtr> mChildren;
    std::vector<std::string> mValues;
    std::vector<Fraction> mWeights;
  };
}
