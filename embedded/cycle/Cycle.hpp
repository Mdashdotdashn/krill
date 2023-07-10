#pragma once

#include <FractionClass/Fraction.hpp>

#include <string>
#include <vector>

namespace krill
{
class Cycle
{
public:
  struct Event
  {
    using Time = Fraction;
    using Value = std::string;

    Event(Time time_, std::vector<Value> values_)
    : time(time_)
    , values(values_)
    {}

    bool operator==(const Event& other) const
    {
      if (other.time != time) return false;
      return std::equal(other.values.begin(), other.values.end(), values.begin());
    }

    bool operator!=(const Event& other) const
    {
      return !this->operator==(other);
    }

    Fraction time;
    std::vector<Value> values;
  };

  Cycle()
  {}

  Cycle(Fraction length_, const std::vector<Event> events_)
  : length(length_)
  , events(events_)
  {}

  Cycle(const std::vector<Event> events_)
  : Cycle(1, events_)
  {}

  bool operator==(const Cycle& other) const
  {
    if (other.length != length)
    {
      return false;
    }
    if (other.events.size() != events.size())
    {
      return false;
    }
    return std::equal(other.events.begin(), other.events.end(), events.begin());
  }

  Fraction length{0};
  std::vector<Event> events{};
};

Cycle makeEmptyCycle();
Cycle makeSingleEventCycle(std::string value);

Cycle concat(const Cycle& c1, const Cycle& c2);
Cycle slice(const Cycle& cycle, const Fraction& from, const Fraction length);
} // namespace krill
