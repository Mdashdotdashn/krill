#include "harmony/core/Interval.hpp"

#include <third_party/catch2/catch.hpp>

using namespace krill::harmony;

TEST_CASE("Harmony Interval")
{
  SECTION("Normalizes pitch classes")
  {
    CHECK(normalizePitchClass(0) == 0);
    CHECK(normalizePitchClass(11) == 11);
    CHECK(normalizePitchClass(12) == 0);
    CHECK(normalizePitchClass(13) == 1);
    CHECK(normalizePitchClass(-1) == 11);
    CHECK(normalizePitchClass(-13) == 11);
  }

  SECTION("Normalizes semitone offsets modulo octave")
  {
    CHECK(normalizeSemitones(0) == 0);
    CHECK(normalizeSemitones(24) == 0);
    CHECK(normalizeSemitones(25) == 1);
    CHECK(normalizeSemitones(-12) == 0);
    CHECK(normalizeSemitones(-14) == 10);
  }

  SECTION("Computes signed semitone distance")
  {
    CHECK(semitoneDistance(60, 67) == 7);
    CHECK(semitoneDistance(67, 60) == -7);
    CHECK(semitoneDistance(42, 42) == 0);
  }

  SECTION("Transposes MIDI in range")
  {
    CHECK(transposeMidi(60, 7).value() == 67);
    CHECK(transposeMidi(60, -12).value() == 48);
    CHECK(transposeMidi(0, 0).value() == 0);
    CHECK(transposeMidi(127, 0).value() == 127);
  }

  SECTION("Rejects out-of-range transposition")
  {
    CHECK(transposeMidi(0, -1) == std::nullopt);
    CHECK(transposeMidi(127, 1) == std::nullopt);
  }
}
