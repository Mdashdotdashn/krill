#pragma once

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "../../harmony/core/NoteMidi.hpp"
#include "../RenderNode.hpp"

namespace krill
{
  class AddRenderNode : public RenderNode
  {
  public:
    AddRenderNode(RenderNodePtr lhs, RenderNodePtr rhs)
      : mpLhs(std::move(lhs)), mpRhs(std::move(rhs))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      const auto lhsFragments = mpLhs->query(request);
      const auto rhsFragments = mpRhs->query(request);
      if (lhsFragments.empty() || rhsFragments.empty())
      {
        return {};
      }

      std::vector<QueryFragment> out;
      for (const auto& rhs : rhsFragments)
      {
        for (const auto& lhs : lhsFragments)
        {
          if (!overlaps(lhs.partStart, lhs.partEnd, rhs.partStart, rhs.partEnd))
          {
            continue;
          }

          const auto overlapStart = lhs.wholeStart > rhs.wholeStart ? lhs.wholeStart : rhs.wholeStart;
          const auto overlapEnd = lhs.wholeEnd < rhs.wholeEnd ? lhs.wholeEnd : rhs.wholeEnd;

          QueryFragment mapped;
          mapped.wholeStart = overlapStart;
          mapped.wholeEnd = overlapEnd;
          mapped.partStart = rhs.partStart;
          mapped.partEnd = rhs.partEnd;
          mapped.value = addValues(lhs.value, rhs.value);
          mapped.controls = rhs.controls;
          out.push_back(mapped);
        }
      }
      return out;
    }

  private:
    bool overlaps(const Fraction& aStart, const Fraction& aEnd, const Fraction& bStart, const Fraction& bEnd) const
    {
      return aStart < bEnd && aEnd > bStart;
    }

    std::optional<double> parseNumber(const std::string& v) const
    {
      try
      {
        size_t idx = 0;
        const double value = std::stod(v, &idx);
        if (idx != v.size())
        {
          return std::nullopt;
        }
        return value;
      }
      catch (...)
      {
        return std::nullopt;
      }
    }

    std::optional<std::string> transposeNote(const std::string& v, int semitones) const
    {
      const auto midi = harmony::noteToMidi(v);
      if (!midi.has_value())
      {
        return std::nullopt;
      }
      const int transposed = midi.value() + semitones;
      if (!harmony::isValidMidi(transposed))
      {
        return std::nullopt;
      }
      return harmony::midiToNote(transposed, harmony::SpellingPolicy::PreferSharps);
    }

    std::string formatDouble(double v) const
    {
      if (std::fabs(v - std::round(v)) < 1e-9)
      {
        return std::to_string(static_cast<long>(std::llround(v)));
      }
      std::string text = std::to_string(v);
      while (!text.empty() && text.back() == '0')
      {
        text.pop_back();
      }
      if (!text.empty() && text.back() == '.')
      {
        text.pop_back();
      }
      return text;
    }

    std::string addValues(const std::string& lhs, const std::string& rhs) const
    {
      const auto lNum = parseNumber(lhs);
      const auto rNum = parseNumber(rhs);
      if (lNum.has_value() && rNum.has_value())
      {
        return formatDouble(lNum.value() + rNum.value());
      }

      if (lNum.has_value() && std::fabs(lNum.value() - std::round(lNum.value())) < 1e-9)
      {
        const auto t = transposeNote(rhs, static_cast<int>(std::llround(lNum.value())));
        if (t.has_value())
        {
          return t.value();
        }
      }

      if (rNum.has_value() && std::fabs(rNum.value() - std::round(rNum.value())) < 1e-9)
      {
        const auto t = transposeNote(lhs, static_cast<int>(std::llround(rNum.value())));
        if (t.has_value())
        {
          return t.value();
        }
      }

      return rhs;
    }

    RenderNodePtr mpLhs;
    RenderNodePtr mpRhs;
  };
}
