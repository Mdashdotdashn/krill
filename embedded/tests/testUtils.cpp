#include "testUtils.hpp"

namespace test
{
Cycle makeCycle(const Fraction& length, const std::vector<std::pair<Fraction, std::string>>& eventDefinition)
{
  Cycle result;
  result.length = length;
  std::transform(eventDefinition.begin(), eventDefinition.end(), std::back_inserter(result.events), [](const std::pair<Fraction, std::string>& ed) { return Cycle::Event(ed.first, { ed.second }); });
  return result;
}

// Makes a cycle of length 1 with even spaced data
Cycle simpleCycle(const std::vector<std::string>& values)
{
  Cycle result;
  result.length = Fraction(1);
  const auto offset = Fraction(1, long(values.size()));
  auto position = Fraction(0);
  for (const auto& s : values)
  {
    result.events.push_back({ position, {s} });
    position += offset;
  }
  return result;
}

bool compare(const Cycle& c1, const Cycle& c2)
{
  if (c1.length != c2.length) return false;
  if (c1.events.size() != c2.events.size()) return false;

  auto it1 = c1.events.begin();
  auto it2 = c2.events.begin();
  while (it1 != c1.events.end())
  {
    if (*it1++ != *it2++) return false;
  }

  return true;
}
}

