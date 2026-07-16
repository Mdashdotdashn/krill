#pragma once

#include "harmony/core/NoteMidi.hpp"

#include <optional>
#include <string>
#include <vector>

namespace krill::harmony
{
struct ScaleDefinition
{
  std::string root;
  std::string name;
  std::vector<int> intervals;
  std::vector<std::string> notes;
};

std::optional<std::vector<int>> scaleIntervals(const std::string& scaleName);
std::optional<std::vector<std::string>> scaleNotes(
  const std::string& root,
  const std::string& scaleName);
std::optional<std::vector<std::string>> scaleNotes(
  const std::string& root,
  const std::string& scaleName,
  SpellingPolicy policy);

std::optional<std::string> scaleDegreeToNote(const ScaleDefinition& scale, int degree);
std::optional<int> scaleDegreeToMidi(
  const ScaleDefinition& scale,
  int degree,
  int rootOctave);

std::optional<ScaleDefinition> parseScale(const std::string& scaleString);
} // namespace krill::harmony
