#include "Cycle.hpp"

#include <algorithm>

namespace krill
{
namespace detail
{
template <typename Function>
Cycle transform(const Cycle& c, const Function& f)
{
  Cycle result = c;
  for (auto& event : result.events)
  {
    event = f(event);
  }
  return result;
}

template <typename Predicate>
Cycle filter(const Cycle& c, const Predicate& p)
{
  Cycle result = c;
  auto& events = result.events;
  auto removeIt = std::remove_if(events.begin(), events.end(), [&p](Cycle::Event& e) { return !p(e); });
  events.erase(removeIt, events.end());
  return result;
}

template <typename TimeFunction>
Cycle applyTimeChange(const Cycle& c, const TimeFunction& f)
{
  Cycle result = c;
  for (auto& event : result.events)
  {
    event.time = f(event.time);
  }
  return result;
}

// Remove all events that don't belong
// to the cycle's length
Cycle cleanup(const Cycle& c)
{
  return detail::filter(c, [&c](const Cycle::Event& e) { return e.time >= 0 && e.time < c.length; });
}
} // namespace detail

// make a cycle out of a single event
Cycle makeSingleEventCycle(const std::string& value)
{
  std::vector<Cycle::Event> eventArray;
  eventArray.push_back({ 0,{value} });
  return Cycle{ eventArray };
}

// concatenate two cycles
Cycle concat(const Cycle& c1, const Cycle& c2)
{
  // c1 comes first
  Cycle result = c1;
  // shift c2's event by c1's length
  const auto shifted = detail::applyTimeChange(c2, [&c1](const Fraction& time) { return time + c1.length; });
  for (const auto &e: shifted.events)
  {
    result.events.push_back(e);
  }
  result.length += c2.length;
  return result;
}

// Return a slice of the repeated cycle
Cycle slice(const Cycle& cycle, const Fraction& from, const Fraction length)
{
  // Make sure we're slicing enough repeat of the original cycles
  Cycle source{};
  while (source.length < (from + length))
  {
    source = concat(source, cycle);
  }
  // Shift all times leftward
  auto shifted = detail::applyTimeChange(source, [&from](const Fraction& time) { return time - from; });
  shifted.length = length;
  return detail::cleanup(shifted);
}

std::optional<Cycle::Event> findEventAfter(const Cycle& cycle, const Fraction& time)
{
  const auto& events = cycle.events;

  if (events.size() > 0)
  {
    for (const auto& event : events)
    {
      // we assume the cycle's events are sorted
      if (event.time > time)
      {
        return event;
      }
    }
  }
  return {};
}

bool isEmpty(const Cycle::Event& event)
{
  return event.values.size() == 0;
}


bool isEmpty(const Cycle& cycle)
{
  return cycle.length == 0;
}

EventArray mergeAndSort(const EventArray& array)
{
  EventArray sorted = array;
  // Sort element according to time
  std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.time < b.time; });
  // Merge steps with equivalent time
  EventArray merged;
  for (const auto& event : sorted)
  {
    // Push any event with a new time
    if ((merged.size() == 0) || (event.time != merged.back().time))
    {
      merged.push_back(event);
    }
    // Add the data to the last event
    else
    {
      for (const auto& value : event.values)
      {
        merged.back().values.push_back(value);
      }
    }
  }  
  return merged;
}

} // namespace krill
