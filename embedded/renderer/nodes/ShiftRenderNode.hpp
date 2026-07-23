#pragma once

#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class ShiftRenderNode : public RenderTree
  {
  public:
    ShiftRenderNode(RenderTreePtr source, Fraction amount, long direction)
      : mpSource(std::move(source)), mAmount(amount), mDirection(direction)
    {
    }

    ShiftRenderNode(RenderTreePtr source, RenderTreePtr amountSource, long direction)
      : mpSource(std::move(source)), mpAmountSource(std::move(amountSource)), mAmount(Fraction(0)), mDirection(direction)
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (!(request.start < request.end))
      {
        return {};
      }

      const auto amount = resolveAmount(request.start);
      const auto delta = amount * Fraction(mDirection, 1);

      QueryRequest shifted;
      shifted.start = request.start - delta;
      shifted.end = request.end - delta;
      const auto sourceFragments = mpSource->query(shifted);

      std::vector<QueryFragment> out;
      out.reserve(sourceFragments.size());
      for (const auto& fragment : sourceFragments)
      {
        QueryFragment mapped;
        mapped.wholeStart = fragment.wholeStart + delta;
        mapped.wholeEnd = fragment.wholeEnd + delta;
        mapped.partStart = fragment.partStart + delta;
        mapped.partEnd = fragment.partEnd + delta;
        mapped.value = fragment.value;
        out.push_back(mapped);
      }
      return out;
    }

  private:
    std::optional<Fraction> parseAmountText(const std::string& text) const
    {
      if (text.find('.') != std::string::npos)
      {
        if (text.empty())
        {
          return std::nullopt;
        }

        size_t pos = 0;
        bool negative = false;
        if (text[pos] == '+' || text[pos] == '-')
        {
          negative = text[pos] == '-';
          pos++;
        }
        if (pos >= text.size())
        {
          return std::nullopt;
        }

        const auto dot = text.find('.', pos);
        const auto intPart = text.substr(pos, dot - pos);
        const auto fracPart = text.substr(dot + 1);

        if (intPart.empty() && fracPart.empty())
        {
          return std::nullopt;
        }

        for (const char c : intPart)
        {
          if (!std::isdigit(static_cast<unsigned char>(c)))
          {
            return std::nullopt;
          }
        }
        for (const char c : fracPart)
        {
          if (!std::isdigit(static_cast<unsigned char>(c)))
          {
            return std::nullopt;
          }
        }

        long whole = 0;
        if (!intPart.empty())
        {
          whole = std::stol(intPart);
        }

        long denom = 1;
        for (size_t i = 0; i < fracPart.size(); i++)
        {
          denom *= 10;
        }

        long frac = 0;
        if (!fracPart.empty())
        {
          frac = std::stol(fracPart);
        }

        long numer = whole * denom + frac;
        if (negative)
        {
          numer = -numer;
        }

        return Fraction(numer, denom);
      }

      try
      {
        return Fraction(text);
      }
      catch (...)
      {
      }
      return std::nullopt;
    }

    Fraction resolveAmount(const Fraction& start) const
    {
      if (!mpAmountSource)
      {
        return mAmount;
      }

      QueryRequest req;
      req.start = start;
      req.end = start + Fraction(1, 1024);
      const auto fragments = mpAmountSource->query(req);
      if (fragments.empty())
      {
        return Fraction(0);
      }

      try
      {
        return Fraction(fragments[0].value);
      }
      catch (...)
      {
        const auto parsed = parseAmountText(fragments[0].value);
        if (parsed.has_value())
        {
          return parsed.value();
        }
        return Fraction(0);
      }
    }

    RenderTreePtr mpSource;
    RenderTreePtr mpAmountSource;
    Fraction mAmount;
    long mDirection;
  };
}
