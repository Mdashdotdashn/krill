#pragma once

#include "third_party/rapidjson/document.h"

#include <functional>
#include <map>
#include <optional>

namespace krill
{
class Context;

enum class ParsingRule
{
	statement,
	command,
	sequence_definition,
	hush,
	setBpm,
};

bool resolve(Context &c, ParsingRule rule);
bool resolveOneOf(Context& c, const std::initializer_list<ParsingRule>& rule);
}
