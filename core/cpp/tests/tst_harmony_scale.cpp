#include "harmony/theory/Scale.hpp"

#include "../third_party/catch2/catch.hpp"

using namespace krill::harmony;

TEST_CASE("Harmony Scale")
{
  SECTION("Looks up major and minor intervals")
  {
    CHECK(scaleIntervals("major").value() == std::vector<int>{0, 2, 4, 5, 7, 9, 11});
    CHECK(scaleIntervals("minor").value() == std::vector<int>{0, 2, 3, 5, 7, 8, 10});
    CHECK(scaleIntervals("Ionian").value() == std::vector<int>{0, 2, 4, 5, 7, 9, 11});
    CHECK(scaleIntervals("aeolian").value() == std::vector<int>{0, 2, 3, 5, 7, 8, 10});
  }

  SECTION("Rejects unknown scales")
  {
    CHECK(scaleIntervals("phrygian") == std::nullopt);
  }

  SECTION("Parses lowercase root scale string from parity baseline")
  {
    const auto scale = parseScale("a minor");
    REQUIRE(scale.has_value());
    CHECK(scale->root == "A");
    CHECK(scale->name == "minor");
    CHECK(scale->intervals == std::vector<int>{0, 2, 3, 5, 7, 8, 10});
    CHECK(scale->notes == std::vector<std::string>{"A", "B", "C", "D", "E", "F", "G"});
  }

  SECTION("Converts scale degrees to notes and MIDI with octave wrapping")
  {
    const auto scale = parseScale("a minor");
    REQUIRE(scale.has_value());

    CHECK(scaleDegreeToNote(*scale, 1).value() == "A");
    CHECK(scaleDegreeToNote(*scale, 7).value() == "G");
    CHECK(scaleDegreeToNote(*scale, 8).value() == "A");
    CHECK(scaleDegreeToNote(*scale, 14).value() == "G");

    CHECK(scaleDegreeToMidi(*scale, 1, 3).value() == 57);
    CHECK(scaleDegreeToMidi(*scale, 7, 3).value() == 67);
    CHECK(scaleDegreeToMidi(*scale, 8, 3).value() == 69);
    CHECK(scaleDegreeToMidi(*scale, 14, 3).value() == 79);
    CHECK(scaleDegreeToMidi(*scale, -1, 3).value() == 53);

    CHECK(scaleDegreeToNote(*scale, 0) == std::nullopt);
    CHECK(scaleDegreeToMidi(*scale, 0, 3) == std::nullopt);
  }

  SECTION("Generates notes with key-signature spelling by default")
  {
    CHECK(scaleNotes("F#", "minor").value()
      == std::vector<std::string>{"F#", "G#", "A", "B", "C#", "D", "E"});

    CHECK(scaleNotes("Bb", "major").value()
      == std::vector<std::string>{"Bb", "C", "D", "Eb", "F", "G", "A"});

    // G# major requires double sharps in theoretical key-signature spelling.
    CHECK(scaleNotes("G#", "major").value()
      == std::vector<std::string>{"G#", "A#", "B#", "C#", "D#", "E#", "F##"});
  }

  SECTION("Allows explicit note spelling override")
  {
    const auto fromSharps = scaleNotes("F#", "minor", SpellingPolicy::PreferSharps);
    const auto fromFlats = scaleNotes("F#", "minor", SpellingPolicy::PreferFlats);
    REQUIRE(fromSharps.has_value());
    REQUIRE(fromFlats.has_value());

    CHECK(*fromSharps == std::vector<std::string>{"F#", "G#", "A", "B", "C#", "D", "E"});
    CHECK(*fromFlats == std::vector<std::string>{"Gb", "Ab", "A", "B", "Db", "D", "E"});
  }

  SECTION("Parses and defaults note spelling by key signature")
  {
    const auto fMajor = parseScale("F major");
    REQUIRE(fMajor.has_value());
    CHECK(fMajor->notes == std::vector<std::string>{"F", "G", "A", "Bb", "C", "D", "E"});

    const auto eMajor = parseScale("E major");
    REQUIRE(eMajor.has_value());
    CHECK(eMajor->notes == std::vector<std::string>{"E", "F#", "G#", "A", "B", "C#", "D#"});
  }

  SECTION("Rejects malformed scale strings")
  {
    CHECK(parseScale("") == std::nullopt);
    CHECK(parseScale("A") == std::nullopt);
    CHECK(parseScale("H minor") == std::nullopt);
    CHECK(parseScale("C weird") == std::nullopt);
    CHECK(parseScale("Cb# major") == std::nullopt);
  }
}
