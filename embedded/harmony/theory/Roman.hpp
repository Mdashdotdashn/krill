#pragma once

#include <optional>
#include <string>

namespace krill::harmony
{
struct RomanNumeral
{
  int degree;
  bool major;
  int accidentalOffset;
  std::string accidental;
};

std::optional<RomanNumeral> parseRomanNumeral(const std::string& token);
} // namespace krill::harmony
