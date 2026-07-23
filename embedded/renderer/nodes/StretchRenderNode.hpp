#pragma once

#include <utility>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class StretchRenderNode final : public RenderTree
  {
  public:
    StretchRenderNode(RenderTreePtr source, Fraction factor)
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
        QueryFragment mapped;
        mapped.wholeStart = childFragment.wholeStart * mFactor;
        mapped.wholeEnd = childFragment.wholeEnd * mFactor;
        mapped.partStart = childFragment.partStart * mFactor;
        mapped.partEnd = childFragment.partEnd * mFactor;
        mapped.value = childFragment.value;
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
    RenderTreePtr mpSource{};
    Fraction mFactor{1};
  };
}
