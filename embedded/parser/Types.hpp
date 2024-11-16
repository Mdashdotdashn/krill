#pragma once

#include "third_party/rapidjson/document.h"

#include <exception>
#include <optional>

namespace krill
{
using ParsingResult = std::optional<rapidjson::Value>;

class ParsingException: public std::exception
{
public:
	ParsingException(): std::exception()
	{
	}
};
} // namespace krill