#include "Parser.hpp"
#include "Context.hpp"
#include "Rules.hpp"

namespace krill
{
void Parser::parse(rapidjson::Document& document, const std::string& input)
{
	Context context(document, input);
	if (!resolve(context, ParsingRule::statement))
	{
		throw ParsingException();
	}
}	
} // krill
