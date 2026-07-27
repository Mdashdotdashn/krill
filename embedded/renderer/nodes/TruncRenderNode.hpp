#pragma once

#include <utility>
#include <vector>

#include "renderer/RenderNode.hpp"

namespace krill
{
  class TruncRenderNode : public RenderNode
  {
  public:
    TruncRenderNode(RenderNodePtr source, Fraction length)
      : mpSource(std::move(source)), mLength(length)
    {
    }

    Fraction spanLength() const override
    {
      return mpSource ? (mpSource->spanLength() * mLength) : mLength;
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (!(request.start < request.end) || mLength <= Fraction(0))
      {
        return {};
      }

      const auto span = spanLength();
      auto startDiv = request.start / span;
      auto endDiv = request.end / span;
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

      std::vector<QueryFragment> out;
      for (auto cycle = startCycle; cycle <= endCycle; cycle++)
      {
        const auto cycleBase = Fraction(cycle) * span;
        const auto cycleEnd = cycleBase + span;
        const auto overlapStart = request.start > cycleBase ? request.start : cycleBase;
        const auto overlapEnd = request.end < cycleEnd ? request.end : cycleEnd;
        if (!(overlapStart < overlapEnd))
        {
          continue;
        }

        QueryRequest local;
        local.start = overlapStart - cycleBase;
        local.end = overlapEnd - cycleBase;
        const auto sourceFragments = mpSource->query(local);
        for (const auto& fragment : sourceFragments)
        {
          QueryFragment mapped;
          mapped.wholeStart = fragment.wholeStart + cycleBase;
          mapped.wholeEnd = fragment.wholeEnd + cycleBase;
          mapped.partStart = fragment.partStart + cycleBase;
          mapped.partEnd = fragment.partEnd + cycleBase;
          mapped.value = fragment.value;
          out.push_back(mapped);
        }
      }

      return out;
    }

  private:
    RenderNodePtr mpSource;
    Fraction mLength;
  };
}
