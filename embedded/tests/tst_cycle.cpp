#include "cycle/Cycle.hpp"

#include <catch2/catch.hpp>

using namespace krill;

TEST_CASE("Cycle")
{
  SECTION("Empty")
  {
    const auto cycle = makeEmptyCycle();
    CHECK(cycle.events.size() == 0);
  }

  SECTION("Single event")
  {
    const auto cycle = makeSingleEventCycle ("C4");
    CHECK(cycle.events.size() == 1);

    const auto event = cycle.events[0];
    CHECK(event.time == Fraction(0));
    CHECK(event.values.size() == 1);
    CHECK(event.values[0] == "C4");
  }
}
