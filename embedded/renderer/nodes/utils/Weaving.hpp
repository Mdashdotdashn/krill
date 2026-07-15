#pragma once

#include "../RenderNode.hpp"

#include <cctype>

namespace krill
{
namespace detail
{
static bool boolValue(const std::string& s)
{
  if (s.empty()) return false;
  const auto lower = [](char c) { return std::tolower(c); };
  const auto first = lower(s[0]);
  return first == 't' || first == '1';
}

static std::vector<std::string> sampleCycle(const Cycle& cycle, Fraction time)
{
  const Cycle::Event* lastEvent = nullptr;

  for (const auto& event : cycle.events)
  {
    if (event.time <= time)
    {
      lastEvent = &event;
    }
    else
    {
      break;
    }
  }

  if (lastEvent)
  {
    return lastEvent->values;
  }

  return {"~"};
}
} // namespace detail
} // namespace krill
