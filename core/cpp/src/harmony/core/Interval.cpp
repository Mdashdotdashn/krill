#include "Interval.hpp"

#include "NoteMidi.hpp"

namespace krill::harmony
{
int normalizePitchClass(int pitchClass)
{
  const int mod = pitchClass % 12;
  return mod < 0 ? mod + 12 : mod;
}

int normalizeSemitones(int semitones)
{
  return normalizePitchClass(semitones);
}

int semitoneDistance(int fromMidi, int toMidi)
{
  return toMidi - fromMidi;
}

std::optional<int> transposeMidi(int midi, int semitoneOffset)
{
  const int out = midi + semitoneOffset;
  if (!isValidMidi(out))
  {
    return std::nullopt;
  }
  return out;
}
} // namespace krill::harmony
