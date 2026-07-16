#include "harmony/core/NoteMidi.hpp"

#include <third_party/catch2/catch.hpp>

using namespace krill::harmony;

TEST_CASE("Harmony NoteMidi")
{
  SECTION("Converts natural notes to MIDI")
  {
    CHECK(noteToMidi("C4").value() == 60);
    CHECK(noteToMidi("A4").value() == 69);
    CHECK(noteToMidi("G3").value() == 55);
    CHECK(noteToMidi("c4").value() == 60);
  }

  SECTION("Converts accidentals deterministically")
  {
    CHECK(noteToMidi("C#4").value() == 61);
    CHECK(noteToMidi("Db4").value() == 61);
    CHECK(noteToMidi("Fb4").value() == 64);
    CHECK(noteToMidi("B#3").value() == 60);
    CHECK(noteToMidi("Ebb4").value() == 62);
  }

  SECTION("Supports MIDI boundary notes")
  {
    CHECK(noteToMidi("C-1").value() == 0);
    CHECK(noteToMidi("G9").value() == 127);
    CHECK(midiToNote(0).value() == "C-1");
    CHECK(midiToNote(127).value() == "G9");
  }

  SECTION("Rejects malformed or out-of-range notes")
  {
    CHECK(noteToMidi("") == std::nullopt);
    CHECK(noteToMidi("H4") == std::nullopt);
    CHECK(noteToMidi("C#") == std::nullopt);
    CHECK(noteToMidi("4C") == std::nullopt);
    CHECK(noteToMidi("Cb#4") == std::nullopt);
    CHECK(noteToMidi("C4x") == std::nullopt);
    CHECK(noteToMidi("C-2") == std::nullopt);
    CHECK(noteToMidi("B9") == std::nullopt);
  }

  SECTION("Alias spellings map to equal MIDI values")
  {
    CHECK(noteToMidi("F#2").value() == noteToMidi("Gb2").value());
    CHECK(noteToMidi("D#6").value() == noteToMidi("Eb6").value());
  }

  SECTION("Converts MIDI to note names with policy")
  {
    CHECK(midiToNote(60).value() == "C4");
    CHECK(midiToNote(61).value() == "C#4");
    CHECK(midiToNote(61, SpellingPolicy::PreferFlats).value() == "Db4");
    CHECK(midiToNote(70).value() == "A#4");
    CHECK(midiToNote(70, SpellingPolicy::PreferFlats).value() == "Bb4");
  }

  SECTION("Round-trip stability with canonical spellings")
  {
    CHECK(noteToMidi(midiToNote(42).value()).value() == 42);
    CHECK(noteToMidi(midiToNote(90).value()).value() == 90);
    CHECK(noteToMidi(midiToNote(90, SpellingPolicy::PreferFlats).value()).value() == 90);
  }

  SECTION("Rejects invalid MIDI numbers for reverse conversion")
  {
    CHECK(midiToNote(-1) == std::nullopt);
    CHECK(midiToNote(128) == std::nullopt);
  }

  SECTION("isValidMidi helper")
  {
    CHECK(isValidMidi(0));
    CHECK(isValidMidi(127));
    CHECK_FALSE(isValidMidi(-1));
    CHECK_FALSE(isValidMidi(128));
  }
}
