#include "Parser.hpp"
#include "Context.hpp"

namespace krill
{
ParsingResult Parser::parse(const std::string& input)
{
	Context context(input);
	if (auto result = resolve(context))
	{
    return result;
	}
	
  throw ParsingException();
}	
} // krill
