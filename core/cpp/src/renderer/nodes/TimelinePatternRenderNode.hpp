#pragma once

#include <utility>

#include "../RenderNode.hpp"

namespace krill
{
  class TimelinePatternRenderNode final : public RenderNode
  {
  public:
    explicit TimelinePatternRenderNode(std::vector<RenderNodePtr> children)
    : mChildren(std::move(children))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end || mChildren.empty())
      {
        return {};
      }

      std::vector<Fraction> spans;
      spans.reserve(mChildren.size());
      Fraction totalSpan(0);
      for (const auto& child : mChildren)
      {
        Fraction span(1);
        if (child)
        {
          span = child->spanLength();
        }
        if (span <= Fraction(0))
        {
          span = Fraction(1);
        }
        spans.push_back(span);
        totalSpan += span;
      }

      if (totalSpan <= Fraction(0))
      {
        return {};
      }

      const auto startDiv = request.start / totalSpan;
      const auto endDiv = request.end / totalSpan;
      auto startCycle = startDiv.getNumerator() / startDiv.getDenominator();
      if (((startDiv.getNumerator() < 0) != (startDiv.getDenominator() < 0))
          && ((startDiv.getNumerator() % startDiv.getDenominator()) != 0))
      {
        startCycle -= 1;
      }
      auto endCycle = endDiv.getNumerator() / endDiv.getDenominator();
      if (((endDiv.getNumerator() < 0) != (endDiv.getDenominator() < 0))
          && ((endDiv.getNumerator() % endDiv.getDenominator()) != 0))
      {
        endCycle -= 1;
      }

      std::vector<QueryFragment> fragments;
      for (auto cycle = startCycle; cycle <= endCycle; cycle++)
      {
        const auto cycleBase = Fraction(cycle) * totalSpan;
        Fraction offset(0);

        for (size_t i = 0; i < mChildren.size(); i++)
        {
          const auto& pChild = mChildren[i];
          const auto slotStart = cycleBase + offset;
          const auto slotEnd = slotStart + spans[i];
          offset += spans[i];

          if (!pChild || request.end <= slotStart || request.start >= slotEnd)
          {
            continue;
          }

          QueryRequest localRequest;
          localRequest.start = (request.start > slotStart ? request.start : slotStart) - slotStart;
          localRequest.end = (request.end < slotEnd ? request.end : slotEnd) - slotStart;

          const auto childFragments = pChild->query(localRequest);
          for (const auto& childFragment : childFragments)
          {
            QueryFragment mapped;
            mapped.wholeStart = childFragment.wholeStart + slotStart;
            mapped.wholeEnd = childFragment.wholeEnd + slotStart;
            mapped.partStart = childFragment.partStart + slotStart;
            mapped.partEnd = childFragment.partEnd + slotStart;
            mapped.value = childFragment.value;
            mapped.controls = childFragment.controls;
            fragments.push_back(mapped);
          }
        }
      }

      return fragments;
    }

  private:
    std::vector<RenderNodePtr> mChildren;
  };
}
