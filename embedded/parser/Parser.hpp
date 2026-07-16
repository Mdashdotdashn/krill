#pragma once

#include "KrillParser.hpp"
#include "Types.hpp"

#include "third_party/rapidjson/document.h"

namespace krill
{
class Parser
{
public:
	ParsingResult parse(rapidjson::Document& document, const std::string& input);
private:
	KrillParser kp_;
};
}