#pragma once

#include "third_party/rapidjson/document.h"

#include <stdexcept>

#include "Rules.hpp"

namespace krill
{
class Parser
{
public:
	ParsingResult parse(rapidjson::Document& document, const std::string& input);
};
}