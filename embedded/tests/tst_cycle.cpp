#include "testUtils.hpp"

TEST_CASE("Cycle")
{
  SECTION("Empty")
  {
    const auto cycle = makeEmptyCycle();
    CHECK(cycle.events.size() == 0);
  }

  SECTION("Single event")
  {
    const auto cycle = makeSingleEventCycle("C4");
    CHECK(cycle.events.size() == 1);

    const auto event = cycle.events[0];
    CHECK(event.time == Fraction(0));
    CHECK(event.values.size() == 1);
    CHECK(event.values[0] == "C4");
  }

  SECTION("Join")
  {
    const auto c1 = test::simpleCycle({ "a" });
    const auto c2 = test::makeCycle(0.5, { {0.25, "b"} });
    const auto expected = test::makeCycle(1.5, { {0, "a"}, {1.25, "b"} });
    const auto result = concat(c1, c2);
    CHECK(result == expected);
  }


  SECTION("slice")
  {
    const auto testSlice = [](const Cycle& cycle, const Fraction& from, const Fraction& length, const Cycle& expected)
    {
      const auto result = slice(cycle, from, length);
      CHECK(result == expected);
    };

    // Standard subcycle
    testSlice(
      test::simpleCycle({ "1", "2", "3", "4" }),
      0.5, 0.5,
      test::makeCycle(0.5, { {0, "3"}, {0.25, "4"} }));

    // if from + length exceedes the original cycle length, the first events are repeated
    testSlice(
      test::simpleCycle({ "1", "2", "3", "4" }),
      0.5, 1,
      test::simpleCycle({ "3", "4", "1", "2" }));

    // Use a full cycle as slice
    testSlice(
      test::simpleCycle({ "1", "2", "3", "4", "1", "2", "3", "4" }),
      0, 1,
      test::simpleCycle({ "1", "2", "3", "4", "1", "2", "3", "4" }));
  }
}