#pragma once

#include <optional>

namespace krill::harmony
{
int normalizePitchClass(int pitchClass);
int normalizeSemitones(int semitones);
int semitoneDistance(int fromMidi, int toMidi);
std::optional<int> transposeMidi(int midi, int semitoneOffset);
} // namespace krill::harmony
