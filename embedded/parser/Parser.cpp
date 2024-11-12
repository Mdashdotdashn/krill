#include "Parser.hpp"

#include "Context.hpp"
#include "Helpers.hpp"

namespace krill
{
ParsingResult Parser::parse(rapidjson::Document& document, const std::string& input)
{
	Context context(document, input);
	if (auto result = resolve(context))
	{
    return result;
	}
	
  throw ParsingException();
}	
} // krill
