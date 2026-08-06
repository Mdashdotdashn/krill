#include "harmony/core/NoteMidi.hpp"

#include <array>
#include <cctype>

namespace krill::harmony
{
namespace
{
int pitchClassFromLetter(char c)
{
  switch (static_cast<char>(std::toupper(c)))
  {
    case 'C': return 0;
    case 'D': return 2;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 7;
    case 'A': return 9;
    case 'B': return 11;
    default: return -1;
  }
}
} // namespace

bool isValidMidi(int midi)
{
  return midi >= 0 && midi <= 127;
}

std::optional<std::string> midiToNote(int midi, SpellingPolicy policy)
{
  static const std::array<const char*, 12> kSharpNames = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
  };
  static const std::array<const char*, 12> kFlatNames = {
    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
  };

  if (!isValidMidi(midi))
  {
    return std::nullopt;
  }

  const int pitchClass = midi % 12;
  const int octave = (midi / 12) - 1;
  const auto& names = (policy == SpellingPolicy::PreferFlats)
    ? kFlatNames
    : kSharpNames;

  return std::string(names[pitchClass]) + std::to_string(octave);
}

std::optional<int> noteToMidi(const std::string& note)
{
  if (note.empty())
  {
    return std::nullopt;
  }

  const int pitchClass = pitchClassFromLetter(note[0]);
  if (pitchClass < 0)
  {
    return std::nullopt;
  }

  size_t idx = 1;
  int accidentalOffset = 0;
  bool sawSharp = false;
  bool sawFlat = false;

  // Parse one or more accidentals before octave (e.g. C#, Ebb).
  while (idx < note.size() && (note[idx] == '#' || note[idx] == 'b'))
  {
    if (note[idx] == '#')
    {
      sawSharp = true;
      accidentalOffset += 1;
    }
    else
    {
      sawFlat = true;
      accidentalOffset -= 1;
    }

    if (sawSharp && sawFlat)
    {
      return std::nullopt;
    }

    ++idx;
  }

  if (idx >= note.size())
  {
    const int midi = 60 + pitchClass + accidentalOffset;
    if (!isValidMidi(midi))
    {
      return std::nullopt;
    }
    return midi;
  }

  bool negativeOctave = false;
  if (note[idx] == '+' || note[idx] == '-')
  {
    negativeOctave = note[idx] == '-';
    ++idx;
  }

  if (idx >= note.size() || !std::isdigit(static_cast<unsigned char>(note[idx])))
  {
    return std::nullopt;
  }

  int octave = 0;
  while (idx < note.size() && std::isdigit(static_cast<unsigned char>(note[idx])))
  {
    octave = octave * 10 + (note[idx] - '0');
    ++idx;
  }

  if (idx != note.size())
  {
    return std::nullopt;
  }

  if (negativeOctave)
  {
    octave = -octave;
  }

  const int midi = (octave + 1) * 12 + pitchClass + accidentalOffset;
  if (!isValidMidi(midi))
  {
    return std::nullopt;
  }

  return midi;
}
} // namespace krill::harmony
