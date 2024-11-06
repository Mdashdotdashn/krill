#pragma once

#include "third_party/rapidjson/document.h"

#include <stdexcept>

namespace krill
{
using ParsingException = std::exception;

class Parser
{
public:
	void parse(rapidjson::Document& document, const std::string& input);
};
}