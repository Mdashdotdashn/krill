#pragma once

#include "Context.hpp"

#include <optional>
#include <string>

namespace krill
{
rapidjson::Value buildXmlForCommand(Context& c, const std::string& command, const std::optional<float> value);
rapidjson::Value buildXmlForPattern(Context& c, rapidjson::Value& sources, const std::string& aligment);
rapidjson::Value buildXmlForElement(Context& c, const std::string& source);
}