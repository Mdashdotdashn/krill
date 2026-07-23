#pragma once

#include <optional>
#include <string>
#include <vector>

#include "harmony/theory/Scale.hpp"
#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class ScaleRenderNode : public RenderTree
  {
  public:
    ScaleRenderNode(std::string scaleName, RenderTreePtr source)
      : mScaleName(std::move(scaleName)), mpSource(std::move(source))
    {
      const auto maybeIntervals = harmony::scaleIntervals(mScaleName);
      if (maybeIntervals.has_value())
      {
        mIntervals = maybeIntervals.value();
      }
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      const auto sourceFragments = mpSource->query(request);
      std::vector<QueryFragment> out;
      out.reserve(sourceFragments.size());

      for (const auto& fragment : sourceFragments)
      {
        QueryFragment mapped = fragment;
        mapped.value = mapValue(fragment.value);
        out.push_back(mapped);
      }

      return out;
    }

  private:
    std::optional<long> parseLongStrict(const std::string& value) const
    {
      try
      {
        size_t idx = 0;
        const long parsed = std::stol(value, &idx);
        if (idx != value.size())
        {
          return std::nullopt;
        }
        return parsed;
      }
      catch (...)
      {
        return std::nullopt;
      }
    }

    long floorDiv(long a, long b) const
    {
      const long q = a / b;
      const long r = a % b;
      return (r != 0 && ((r < 0) != (b < 0))) ? (q - 1) : q;
    }

    long positiveMod(long a, long b) const
    {
      const long m = a % b;
      return m < 0 ? m + b : m;
    }

    std::string mapValue(const std::string& value) const
    {
      if (mIntervals.empty())
      {
        return value;
      }

      const auto degree = parseLongStrict(value);
      if (!degree.has_value())
      {
        return value;
      }

      const auto size = static_cast<long>(mIntervals.size());
      const auto octave = floorDiv(degree.value(), size);
      const auto index = positiveMod(degree.value(), size);
      const auto semitone = octave * 12 + mIntervals[static_cast<size_t>(index)];
      return std::to_string(semitone);
    }

    std::string mScaleName;
    std::vector<int> mIntervals;
    RenderTreePtr mpSource;
  };
}
