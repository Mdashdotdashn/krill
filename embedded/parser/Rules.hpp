#pragma once

#include "Context.hpp"

#include "third_party/rapidjson/document.h"

#include <functional>
#include <map>
#include <optional>

namespace krill
{
using ParsingResult = std::optional<rapidjson::Value>;
ParsingResult resolve(Context& c);
}
