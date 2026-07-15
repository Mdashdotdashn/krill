#pragma once

#include "../RenderNode.hpp"

#include <cctype>
#include <map>

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

static EventArray mergeEventsByTime(const EventArray& events)
{
  std::map<Fraction, std::vector<std::string>> merged;
  for (const auto& event : events)
  {
    merged[event.time].insert(merged[event.time].end(), event.values.begin(), event.values.end());
  }

  EventArray result;
  for (const auto& [time, values] : merged)
  {
    result.push_back(Cycle::Event(time, values));
  }

  return result;
}
} // namespace detail
} // namespace krill
