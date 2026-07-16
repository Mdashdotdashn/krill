#pragma once

#include <optional>
#include <string>

namespace krill::harmony
{
std::optional<int> noteToMidi(const std::string& note);
bool isValidMidi(int midi);
} // namespace krill::harmony
