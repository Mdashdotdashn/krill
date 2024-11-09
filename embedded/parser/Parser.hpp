#pragma once

#include "third_party/rapidjson/document.h"

#include <stdexcept>

#include "Rules.hpp"

namespace krill
{
class Parser
{
public:
	ParsingResult parse(const std::string& input);
};
}