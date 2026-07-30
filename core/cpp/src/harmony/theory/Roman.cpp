#include "harmony/theory/Roman.hpp"

#include <cctype>

namespace krill::harmony
{
namespace
{
bool isRomanLetter(char c)
{
  const char lc = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  return lc == 'i' || lc == 'v';
}

bool hasOnlyCase(const std::string& s, bool upper)
{
  for (const char c : s)
  {
    if (!isRomanLetter(c))
    {
      return false;
    }

    if (upper && !std::isupper(static_cast<unsigned char>(c)))
    {
      return false;
    }
    if (!upper && !std::islower(static_cast<unsigned char>(c)))
    {
      return false;
    }
  }
  return !s.empty();
}

int romanToDegree(const std::string& roman)
{
  if (roman == "I") return 1;
  if (roman == "II") return 2;
  if (roman == "III") return 3;
  if (roman == "IV") return 4;
  if (roman == "V") return 5;
  if (roman == "VI") return 6;
  if (roman == "VII") return 7;
  return 0;
}

bool parseAccidentals(const std::string& s, std::string& accidental, int& offset)
{
  accidental.clear();
  offset = 0;
  bool sawSharp = false;
  bool sawFlat = false;

  for (const char c : s)
  {
    if (c != '#' && c != 'b')
    {
      return false;
    }

    accidental.push_back(c);
    if (c == '#')
    {
      sawSharp = true;
      ++offset;
    }
    else
    {
      sawFlat = true;
      --offset;
    }

    if (sawSharp && sawFlat)
    {
      return false;
    }
  }

  return true;
}
} // namespace

std::optional<RomanNumeral> parseRomanNumeral(const std::string& token)
{
  if (token.empty())
  {
    return std::nullopt;
  }

  size_t prefixLen = 0;
  while (prefixLen < token.size() && (token[prefixLen] == '#' || token[prefixLen] == 'b'))
  {
    ++prefixLen;
  }

  size_t suffixStart = token.size();
  while (suffixStart > prefixLen && (token[suffixStart - 1] == '#' || token[suffixStart - 1] == 'b'))
  {
    --suffixStart;
  }

  if (prefixLen > 0 && suffixStart < token.size())
  {
    return std::nullopt;
  }

  const std::string prefixAcc = token.substr(0, prefixLen);
  const std::string suffixAcc = token.substr(suffixStart);
  const std::string accidentalRaw = prefixLen > 0 ? prefixAcc : suffixAcc;
  const std::string romanPart = token.substr(prefixLen, suffixStart - prefixLen);

  if (romanPart.empty())
  {
    return std::nullopt;
  }

  const bool isUpper = hasOnlyCase(romanPart, true);
  const bool isLower = hasOnlyCase(romanPart, false);
  if (!isUpper && !isLower)
  {
    return std::nullopt;
  }

  std::string accidental;
  int accidentalOffset = 0;
  if (!parseAccidentals(accidentalRaw, accidental, accidentalOffset))
  {
    return std::nullopt;
  }

  std::string upperRoman = romanPart;
  for (char& c : upperRoman)
  {
    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }

  const int degree = romanToDegree(upperRoman);
  if (degree == 0)
  {
    return std::nullopt;
  }

  RomanNumeral out;
  out.degree = degree;
  out.major = isUpper;
  out.accidentalOffset = accidentalOffset;
  out.accidental = accidental;
  return out;
}
} // namespace krill::harmony
