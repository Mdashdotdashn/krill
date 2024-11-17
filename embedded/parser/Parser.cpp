#include "Parser.hpp"

#include "Context.hpp"
#include "Helpers.hpp"
#include "MiniNotation.hpp"
#include "Types.hpp"
#include "XmlBuilder.hpp"

#include <cassert>
#include <functional>
#include <regex>
#include <string>

using Context = krill::Context;
using ParsingResult = krill::ParsingResult;
using ParsingException = krill::ParsingException;
using Value = rapidjson::Value;

ParsingResult resolveOneOf(Context& context, const std::initializer_list<std::function<ParsingResult(Context&)>>& handlers)
{
  for (auto& handler: handlers)
  {
    if (auto result = handler(context))
    {
      return result;
    }
  }
  return {};
}

ParsingResult parseSequenceDefinition(Context& c)
{
  return parseMiniNotation(c);
}

ParsingResult parseHush(Context& c)
{
  const std::string token("hush");
  if (c.consumeToken(token))
  {
    return buildXmlForCommand(c, token, {});
  }
  return {};
}

ParsingResult parseSetCps(Context& c)
{
  const std::string token("setcps");
  if (c.consumeToken(token))
  {
      if (auto s = c.consumeFloat())
      {
        const auto f = std::stof(s.value());
        return buildXmlForCommand(c, token, f);
      }
      throw ParsingException();
  }
  return {};
}

ParsingResult parseSetBpm(Context& c)
{
  if (c.consumeToken("setbpm"))
  {
      if (auto s = c.consumeFloat())
      {
        const auto f = std::stof(s.value());
        return buildXmlForCommand(c, "setcps", f/120.f/2.f);
      }
      throw ParsingException();
  }
  return {};
}


ParsingResult parseCommand(Context& c)
{
  return resolveOneOf(c, {
    parseHush,
    parseSetBpm,
    parseSetCps,
    });
}

namespace krill
{
ParsingResult Parser::parse(rapidjson::Document& document, const std::string& input)
{
	Context context(document, input);

  auto result = resolveOneOf(context, {
    parseCommand,
    parseSequenceDefinition,
    });

	if (! result)
	{
    throw ParsingException();
	}
	return result;
}	
} // krill
