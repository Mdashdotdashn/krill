#include "harmony/theory/Scale.hpp"

#include "harmony/core/Interval.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <sstream>
#include <unordered_map>

namespace krill::harmony
{
namespace
{
int naturalPitchClassFromLetter(char c)
{
  switch (c)
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

int letterIndex(char c)
{
  switch (c)
  {
    case 'C': return 0;
    case 'D': return 1;
    case 'E': return 2;
    case 'F': return 3;
    case 'G': return 4;
    case 'A': return 5;
    case 'B': return 6;
    default: return -1;
  }
}

char letterFromIndex(int idx)
{
  static const char kLetters[7] = {'C', 'D', 'E', 'F', 'G', 'A', 'B'};
  return kLetters[idx % 7];
}

std::string accidentalString(int offset)
{
  if (offset == 0)
  {
    return "";
  }

  const char accidental = offset > 0 ? '#' : 'b';
  const int count = offset > 0 ? offset : -offset;
  return std::string(static_cast<size_t>(count), accidental);
}

int chooseAccidentalOffset(int desiredPc, int naturalPc, SpellingPolicy policy)
{
  int delta = desiredPc - naturalPc;
  while (delta < -6)
  {
    delta += 12;
  }
  while (delta > 6)
  {
    delta -= 12;
  }

  if (delta == 6 && policy == SpellingPolicy::PreferFlats)
  {
    return -6;
  }
  if (delta == -6 && policy == SpellingPolicy::PreferSharps)
  {
    return 6;
  }
  return delta;
}

std::string toLower(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return s;
}

std::optional<std::string> normalizeRoot(const std::string& rawRoot)
{
  if (rawRoot.empty())
  {
    return std::nullopt;
  }

  const char letter = static_cast<char>(std::toupper(static_cast<unsigned char>(rawRoot[0])));
  if (letter < 'A' || letter > 'G')
  {
    return std::nullopt;
  }

  std::string suffix;
  bool sawSharp = false;
  bool sawFlat = false;
  for (size_t i = 1; i < rawRoot.size(); ++i)
  {
    const char c = rawRoot[i];
    if (c != '#' && c != 'b')
    {
      return std::nullopt;
    }

    suffix.push_back(c);
    sawSharp = sawSharp || (c == '#');
    sawFlat = sawFlat || (c == 'b');
    if (sawSharp && sawFlat)
    {
      return std::nullopt;
    }
  }

  return std::string(1, letter) + suffix;
}

bool prefersFlatsByKeySignature(const std::string& normalizedRoot, const std::string& normalizedScaleName)
{
  static const std::array<const char*, 7> kMajorFlats = {
    "F", "Bb", "Eb", "Ab", "Db", "Gb", "Cb"
  };
  static const std::array<const char*, 7> kMinorFlats = {
    "D", "G", "C", "F", "Bb", "Eb", "Ab"
  };

  if (normalizedRoot.find('b') != std::string::npos)
  {
    return true;
  }
  if (normalizedRoot.find('#') != std::string::npos)
  {
    return false;
  }

  const auto inSet = [&](const auto& keys) {
    return std::find_if(keys.begin(), keys.end(), [&](const char* k) {
      return normalizedRoot == k;
    }) != keys.end();
  };

  if (normalizedScaleName == "major")
  {
    return inSet(kMajorFlats);
  }
  if (normalizedScaleName == "minor")
  {
    return inSet(kMinorFlats);
  }

  return false;
}

std::optional<int> rootPitchClass(const std::string& normalizedRoot)
{
  const auto midi = noteToMidi(normalizedRoot + "4");
  if (!midi.has_value())
  {
    return std::nullopt;
  }
  return normalizePitchClass(*midi);
}

int floorDiv(int a, int b)
{
  const int q = a / b;
  const int r = a % b;
  return (r != 0 && ((r < 0) != (b < 0))) ? (q - 1) : q;
}

int positiveMod(int a, int b)
{
  const int m = a % b;
  return m < 0 ? m + b : m;
}
} // namespace

std::optional<std::vector<int>> scaleIntervals(const std::string& scaleName)
{
  static const std::unordered_map<std::string, std::vector<int>> kIntervals = {
    {"major", {0, 2, 4, 5, 7, 9, 11}},
    {"ionian", {0, 2, 4, 5, 7, 9, 11}},
    {"minor", {0, 2, 3, 5, 7, 8, 10}},
    {"aeolian", {0, 2, 3, 5, 7, 8, 10}}
  };

  const auto normalized = toLower(scaleName);
  const auto it = kIntervals.find(normalized);
  if (it == kIntervals.end())
  {
    return std::nullopt;
  }
  return it->second;
}

std::optional<std::vector<std::string>> scaleNotes(
  const std::string& root,
  const std::string& scaleName)
{
  const auto normalizedRoot = normalizeRoot(root);
  if (!normalizedRoot.has_value())
  {
    return std::nullopt;
  }

  const auto normalizedScaleName = toLower(scaleName);
  const auto intervals = scaleIntervals(normalizedScaleName);
  if (!intervals.has_value())
  {
    return std::nullopt;
  }

  const auto rootPc = rootPitchClass(*normalizedRoot);
  if (!rootPc.has_value())
  {
    return std::nullopt;
  }

  const SpellingPolicy deducedPolicy =
    prefersFlatsByKeySignature(*normalizedRoot, normalizedScaleName)
      ? SpellingPolicy::PreferFlats
      : SpellingPolicy::PreferSharps;

  std::vector<std::string> out;
  out.reserve(intervals->size());

  const int rootLetterIdx = letterIndex((*normalizedRoot)[0]);
  if (rootLetterIdx < 0)
  {
    return std::nullopt;
  }

  for (size_t i = 0; i < intervals->size(); ++i)
  {
    const int targetPc = normalizePitchClass(*rootPc + (*intervals)[i]);
    const char letter = letterFromIndex(rootLetterIdx + static_cast<int>(i));
    const int naturalPc = naturalPitchClassFromLetter(letter);
    if (naturalPc < 0)
    {
      return std::nullopt;
    }

    const int accidentalOffset = chooseAccidentalOffset(targetPc, naturalPc, deducedPolicy);
    out.push_back(std::string(1, letter) + accidentalString(accidentalOffset));
  }

  return out;
}

std::optional<std::vector<std::string>> scaleNotes(
  const std::string& root,
  const std::string& scaleName,
  SpellingPolicy policy)
{
  const auto normalizedRoot = normalizeRoot(root);
  if (!normalizedRoot.has_value())
  {
    return std::nullopt;
  }

  const auto intervals = scaleIntervals(scaleName);
  if (!intervals.has_value())
  {
    return std::nullopt;
  }

  const auto rootPc = rootPitchClass(*normalizedRoot);
  if (!rootPc.has_value())
  {
    return std::nullopt;
  }

  std::vector<std::string> out;
  out.reserve(intervals->size());
  for (size_t i = 0; i < intervals->size(); ++i)
  {
    const int midi = 60 + normalizePitchClass(*rootPc + (*intervals)[i]);
    const auto nameWithOctave = midiToNote(midi, policy);
    if (!nameWithOctave.has_value())
    {
      return std::nullopt;
    }

    auto noteName = *nameWithOctave;
    while (!noteName.empty() && std::isdigit(static_cast<unsigned char>(noteName.back())))
    {
      noteName.pop_back();
    }
    out.push_back(noteName);
  }

  return out;
}

std::optional<std::string> scaleDegreeToNote(const ScaleDefinition& scale, int degree)
{
  if (degree == 0 || scale.notes.empty() || scale.notes.size() != scale.intervals.size())
  {
    return std::nullopt;
  }

  const int n = static_cast<int>(scale.notes.size());
  const int zeroBased = degree - 1;
  const int idx = positiveMod(zeroBased, n);
  return scale.notes[static_cast<size_t>(idx)];
}

std::optional<int> scaleDegreeToMidi(
  const ScaleDefinition& scale,
  int degree,
  int rootOctave)
{
  if (degree == 0 || scale.intervals.empty() || scale.notes.size() != scale.intervals.size())
  {
    return std::nullopt;
  }

  const auto baseMidi = noteToMidi(scale.root + std::to_string(rootOctave));
  if (!baseMidi.has_value())
  {
    return std::nullopt;
  }

  const int n = static_cast<int>(scale.intervals.size());
  const int zeroBased = degree - 1;
  const int octaveShift = floorDiv(zeroBased, n);
  const int idx = positiveMod(zeroBased, n);

  const int midi = *baseMidi + scale.intervals[static_cast<size_t>(idx)] + (12 * octaveShift);
  if (!isValidMidi(midi))
  {
    return std::nullopt;
  }
  return midi;
}

std::optional<ScaleDefinition> parseScale(const std::string& scaleString)
{
  std::istringstream iss(scaleString);
  std::string rawRoot;
  if (!(iss >> rawRoot))
  {
    return std::nullopt;
  }

  std::string rawName;
  if (!(iss >> rawName))
  {
    return std::nullopt;
  }

  std::string extra;
  while (iss >> extra)
  {
    rawName += " " + extra;
  }

  const auto normalizedRoot = normalizeRoot(rawRoot);
  if (!normalizedRoot.has_value())
  {
    return std::nullopt;
  }

  const auto normalizedName = toLower(rawName);
  const auto intervals = scaleIntervals(normalizedName);
  if (!intervals.has_value())
  {
    return std::nullopt;
  }

  const auto notes = scaleNotes(*normalizedRoot, normalizedName);
  if (!notes.has_value())
  {
    return std::nullopt;
  }

  ScaleDefinition def;
  def.root = *normalizedRoot;
  def.name = normalizedName;
  def.intervals = *intervals;
  def.notes = *notes;
  return def;
}
} // namespace krill::harmony
