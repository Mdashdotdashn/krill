#pragma once

#include <optional>
#include <string>

namespace krill::harmony
{
enum class SpellingPolicy
{
	PreferSharps,
	PreferFlats
};

std::optional<int> noteToMidi(const std::string& note);
std::optional<std::string> midiToNote(
	int midi,
	SpellingPolicy policy = SpellingPolicy::PreferSharps);
bool isValidMidi(int midi);
} // namespace krill::harmony
