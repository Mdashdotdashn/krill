#pragma once

#include "Context.hpp"
#include "Types.hpp"

namespace krill
{
// Expects a mini notation quoted string
// "[bd] a <1, 2,3>%1, 4"
ParsingResult parseMiniNotation(Context& c);
} // namespace krill
