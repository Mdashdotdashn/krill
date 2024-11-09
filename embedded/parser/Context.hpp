#pragma once

#include "StringStream.hpp"

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
	Context(const std::string& source);

	bool consumeToken(const std::string& token);
	std::optional<std::string> consumeFloat();

  rapidjson::Document& document();

private:
	rapidjson::Document mDocument{};
	StringStream mStream;
};

} // namespace krill