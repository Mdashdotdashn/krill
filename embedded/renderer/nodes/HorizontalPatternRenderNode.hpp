#pragma once

#include <utility>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class HorizontalPatternRenderNode final : public RenderTree
  {
  public:
    explicit HorizontalPatternRenderNode(std::vector<std::string> values)
    : mValues(std::move(values))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end || mValues.empty())
      {
        return {};
      }

      std::vector<QueryFragment> fragments;
      const auto count = static_cast<long>(mValues.size());

      for (long i = 0; i < count; i++)
      {
        const auto wholeStart = Fraction(i, count);
        const auto wholeEnd = Fraction(i + 1, count);
        if (request.end <= wholeStart || request.start >= wholeEnd)
        {
          continue;
        }

        QueryFragment fragment;
        fragment.wholeStart = wholeStart;
        fragment.wholeEnd = wholeEnd;
        fragment.partStart = request.start > wholeStart ? request.start : wholeStart;
        fragment.partEnd = request.end < wholeEnd ? request.end : wholeEnd;
        fragment.value = mValues[static_cast<size_t>(i)];
        fragments.push_back(fragment);
      }

      return fragments;
    }

  private:
    std::vector<std::string> mValues;
  };
}
