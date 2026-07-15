#pragma once

#include "../RenderNode.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace krill
{
namespace detail
{
static std::pair<bool, double> tryParseDouble(const std::string& s)
{
  try
  {
    const auto result = std::stod(s);
    return {true, result};
  }
  catch (...)
  {
    return {false, 0.0};
  }
}

static std::string formatDouble(double value)
{
  if (value == static_cast<long long>(value))
  {
    return std::to_string(static_cast<long long>(value));
  }

  char buffer[32];
  std::snprintf(buffer, sizeof(buffer), "%.15g", value);
  return std::string(buffer);
}

static std::string addValues(const std::string& left, const std::string& right)
{
  if (left == "~" || right == "~")
  {
    return "~";
  }

  const auto [leftOk, leftVal] = detail::tryParseDouble(left);
  const auto [rightOk, rightVal] = detail::tryParseDouble(right);

  if (!leftOk || !rightOk)
  {
    return "~";
  }

  const auto result = leftVal + rightVal;
  return detail::formatDouble(result);
}

static std::vector<Fraction> collectEventTimes(const Cycle& left, const Cycle& right)
{
  std::vector<Fraction> times;
  times.reserve(left.events.size() + right.events.size());

  for (const auto& e : left.events)
  {
    times.push_back(e.time);
  }
  for (const auto& e : right.events)
  {
    times.push_back(e.time);
  }

  std::sort(times.begin(), times.end());
  times.erase(std::unique(times.begin(), times.end()), times.end());

  return times;
}
} // namespace detail
} // namespace krill
