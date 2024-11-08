#pragma once

#include "StringStream.hpp"
#include "Rules.hpp"

#include "third_party/rapidjson/document.h"

#include <initializer_list>
#include <string>
#include <sstream>
#include <iostream>

namespace krill
{
class Context
{
public:
	Context(rapidjson::Document& document, const std::string& source);

	bool consumeToken(const std::string& token);
	std::optional<std::string> consumeFloat();
	void addCommand(const std::string& command, const std::optional<float> value);

private:
	rapidjson::Document& mDocument;
	StringStream mStream;
};

} // namespace krill