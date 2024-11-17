#pragma once

#include "Types.hpp"

#include "third_party/rapidjson/document.h"

#include <stdexcept>

namespace krill
{
class Parser
{
public:
	ParsingResult parse(rapidjson::Document& document, const std::string& input);
};
}