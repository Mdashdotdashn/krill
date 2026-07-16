#pragma once

#include "StringStream.hpp"

#include "third_party/rapidjson/document.h"

#include <initializer_list>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

namespace krill
{
class Context
{
public:
	Context(rapidjson::Document& document, const std::string& source);

	bool consumeToken(const std::string& token);
	std::optional<std::string> consumeFloat();
	std::optional<std::string> consumeDelimitedString(const std::string& delimiters);
	std::vector<std::string> consumeTokens(const std::string& separators);
  rapidjson::Document& document();

	Context buildSubContext(std::string& subString);

private:
	StringStream mStream;
	rapidjson::Document& mDocument;
};

} // namespace krill