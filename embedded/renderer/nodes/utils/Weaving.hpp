#pragma once

#include "../RenderNode.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>

namespace krill
{
namespace detail
{
enum class WeaveSamplingMode
{
  left,
  right,
  both,
};

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

static std::vector<Fraction> collectSamplingTimes(const Cycle& left,
                                                  const Cycle& right,
                                                  WeaveSamplingMode mode)
{
  std::vector<Fraction> times;

  const auto appendTimes = [&](const Cycle& cycle) {
    for (const auto& e : cycle.events)
    {
      times.push_back(e.time);
    }
  };

  if (mode == WeaveSamplingMode::left)
  {
    appendTimes(left);
  }
  else if (mode == WeaveSamplingMode::right)
  {
    appendTimes(right);
  }
  else
  {
    appendTimes(left);
    appendTimes(right);
  }

  std::sort(times.begin(), times.end());
  times.erase(std::unique(times.begin(), times.end()), times.end());
  return times;
}

static Fraction fractionFromValueOrZero(const std::string& value)
{
  char* end = nullptr;
  const auto parsed = std::strtod(value.c_str(), &end);
  if (end && *end == '\0')
  {
    Fraction result;
    result.convertDoubleToFraction(parsed);
    return result;
  }

  try
  {
    Fraction result;
    result.convertStringToFraction(value);
    return result;
  }
  catch (...)
  {
    return Fraction(0);
  }
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

template<typename Operation>
static Cycle weaveCycles(const Cycle& left,
                         const Cycle& right,
                         WeaveSamplingMode mode,
                         Operation operation)
{
  EventArray events;
  const auto sampleTimes = collectSamplingTimes(left, right, mode);

  for (const auto& time : sampleTimes)
  {
    const auto leftValues = sampleCycle(left, time);
    const auto rightValues = sampleCycle(right, time);

    std::vector<std::string> values;
    values.reserve(leftValues.size() * rightValues.size());
    for (const auto& leftValue : leftValues)
    {
      for (const auto& rightValue : rightValues)
      {
        values.push_back(operation(leftValue, rightValue));
      }
    }

    events.push_back(Cycle::Event(time, std::move(values)));
  }

  return {Fraction(1), mergeEventsByTime(events)};
}
} // namespace detail
} // namespace krill
