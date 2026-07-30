#include "harmony/theory/Roman.hpp"

#include "../third_party/catch2/catch.hpp"

using namespace krill::harmony;

TEST_CASE("Harmony Roman Numeral")
{
  SECTION("Parses quality from case")
  {
    const auto major = parseRomanNumeral("VII");
    REQUIRE(major.has_value());
    CHECK(major->degree == 7);
    CHECK(major->major);
    CHECK(major->accidentalOffset == 0);
    CHECK(major->accidental == "");

    const auto minor = parseRomanNumeral("vii");
    REQUIRE(minor.has_value());
    CHECK(minor->degree == 7);
    CHECK_FALSE(minor->major);
    CHECK(minor->accidentalOffset == 0);
    CHECK(minor->accidental == "");
  }

  SECTION("Parses accidentals")
  {
    const auto suffixFlat = parseRomanNumeral("IIb");
    REQUIRE(suffixFlat.has_value());
    CHECK(suffixFlat->degree == 2);
    CHECK(suffixFlat->major);
    CHECK(suffixFlat->accidentalOffset == -1);
    CHECK(suffixFlat->accidental == "b");

    const auto prefixSharp = parseRomanNumeral("#iv");
    REQUIRE(prefixSharp.has_value());
    CHECK(prefixSharp->degree == 4);
    CHECK_FALSE(prefixSharp->major);
    CHECK(prefixSharp->accidentalOffset == 1);
    CHECK(prefixSharp->accidental == "#");

    const auto doubleFlat = parseRomanNumeral("bbIII");
    REQUIRE(doubleFlat.has_value());
    CHECK(doubleFlat->degree == 3);
    CHECK(doubleFlat->major);
    CHECK(doubleFlat->accidentalOffset == -2);
    CHECK(doubleFlat->accidental == "bb");
  }

  SECTION("Rejects malformed numerals")
  {
    CHECK(parseRomanNumeral("") == std::nullopt);
    CHECK(parseRomanNumeral("Ii") == std::nullopt);
    CHECK(parseRomanNumeral("VIII") == std::nullopt);
    CHECK(parseRomanNumeral("IIb#") == std::nullopt);
    CHECK(parseRomanNumeral("x") == std::nullopt);
    CHECK(parseRomanNumeral("3") == std::nullopt);
  }
}
