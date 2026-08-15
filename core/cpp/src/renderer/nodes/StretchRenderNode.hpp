#pragma once

#include <utility>

#include "../RenderNode.hpp"

namespace krill
{
  class StretchRenderNode final : public RenderNode
  {
  public:
    StretchRenderNode(RenderNodePtr source, Fraction factor)
    : mpSource(std::move(source)), mFactor(std::move(factor))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end || !mpSource || mFactor == Fraction(0))
      {
        return {};
      }

      QueryRequest localRequest;
      localRequest.start = request.start / mFactor;
      localRequest.end = request.end / mFactor;

      const auto childFragments = mpSource->query(localRequest);
      std::vector<QueryFragment> fragments;
      fragments.reserve(childFragments.size());
      for (const auto& childFragment : childFragments)
      {
        auto mapped = remapFragmentTiming(
          childFragment,
          childFragment.wholeStart * mFactor,
          childFragment.wholeEnd * mFactor,
          childFragment.partStart * mFactor,
          childFragment.partEnd * mFactor);
        fragments.push_back(mapped);
      }

      return fragments;
    }

    Fraction spanLength() const override
    {
      if (!mpSource)
      {
        return mFactor;
      }
      return mpSource->spanLength() * mFactor;
    }

  private:
    RenderNodePtr mpSource{};
    Fraction mFactor{1};
  };
}
