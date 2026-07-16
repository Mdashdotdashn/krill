#include "renderer/nodes/utils/Weaving.hpp"

#include "testUtils.hpp"

TEST_CASE("weaveCycles uses left sampling structure")
{
  const auto left = test::simpleCycle({"1", "2"});
  const auto right = test::simpleCycle({"a", "b", "c", "d"});

  const auto result = krill::detail::weaveCycles(left,
                                                 right,
                                                 krill::detail::WeaveSamplingMode::left,
                                                 [](const std::string& leftValue, const std::string&) {
                                                   return leftValue;
                                                 });

  const auto expected = test::makeCycle(Fraction(1), {
      {Fraction(0), "1"},
      {Fraction(1, 2), "2"},
  });

  CHECK(test::compare(expected, result));
}

TEST_CASE("weaveCycles can use left structure and right data")
{
  const auto left = test::simpleCycle({"1", "2"});
  const auto right = test::simpleCycle({"a", "b", "c", "d"});

  const auto result = krill::detail::weaveCycles(left,
                                                 right,
                                                 krill::detail::WeaveSamplingMode::left,
                                                 [](const std::string&, const std::string& rightValue) {
                                                   return rightValue;
                                                 });

  const auto expected = test::makeCycle(Fraction(1), {
      {Fraction(0), "a"},
      {Fraction(1, 2), "c"},
  });

  CHECK(test::compare(expected, result));
}

TEST_CASE("weaveCycles uses right sampling structure")
{
  const auto left = test::simpleCycle({"1", "2"});
  const auto right = test::simpleCycle({"a", "b", "c", "d"});

  const auto result = krill::detail::weaveCycles(left,
                                                 right,
                                                 krill::detail::WeaveSamplingMode::right,
                                                 [](const std::string& leftValue, const std::string&) {
                                                   return leftValue;
                                                 });

  const auto expected = test::makeCycle(Fraction(1), {
      {Fraction(0), "1"},
      {Fraction(1, 4), "1"},
      {Fraction(1, 2), "2"},
      {Fraction(3, 4), "2"},
  });

  CHECK(test::compare(expected, result));
}

TEST_CASE("weaveCycles uses both sampling structures")
{
  const auto left = test::simpleCycle({"2", "3"});
  const auto right = test::simpleCycle({"4", "5", "6"});

  const auto result = krill::detail::weaveCycles(left,
                                                 right,
                                                 krill::detail::WeaveSamplingMode::both,
                                                 [](const std::string& leftValue, const std::string& rightValue) {
                                                   return leftValue + rightValue;
                                                 });

  const auto expected = test::makeCycle(Fraction(1), {
      {Fraction(0), "24"},
      {Fraction(1, 3), "25"},
      {Fraction(1, 2), "35"},
      {Fraction(2, 3), "36"},
  });

  CHECK(test::compare(expected, result));
}