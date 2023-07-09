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

  Fraction length{0};
  std::vector<Event> events{};
};

Cycle makeEmptyCycle();
Cycle makeSingleEventCycle(std::string value);
} // namespace krill
