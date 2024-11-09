#include "Rules.hpp"
#include "Context.hpp"

#include <cassert>
#include <string>

namespace
{
	using Context = krill::Context;
	using ParsingRule = krill::ParsingRule;
	using ParsingException = krill::ParsingException;

	bool parseStart(Context& c)
	{
		return resolveOneOf(c, {
			ParsingRule::command,
			ParsingRule::sequence_definition,
			});
	}

	bool parseCommand(Context& c)
	{
		return resolveOneOf(c, {
			ParsingRule::hush,
			ParsingRule::setBpm,
			ParsingRule::setCps,
			});
	}

	bool parseSequenceDefinition(Context& c)
	{
		return resolve(c, ParsingRule::single_cycle);
	}

	bool parseSingleCycle(Context& c)
	{
    return false;
	}

	bool parseHush(Context& c)
	{
		const std::string token("hush");
		if (c.consumeToken(token))
		{
			c.addCommand(token, {});
			return true;
		}
		return false;
	}

	bool parseSetCps(Context& c)
	{
		const std::string token("setcps");
		if (c.consumeToken(token))
		{
				if (auto s = c.consumeFloat())
				{
					const auto f = std::stof(s.value());
					c.addCommand(token, f);
					return true;
				}
				throw ParsingException();
		}
		return false;
	}

	bool parseSetBpm(Context& c)
	{
		if (c.consumeToken("setbpm"))
		{
				if (auto s = c.consumeFloat())
				{
					const auto f = std::stof(s.value());
					c.addCommand("setcps", f/120.f/2.f);
					return true;
				}
				throw ParsingException();
		}
		return false;
	}
}

namespace krill
{
	bool resolve(Context& context, ParsingRule rule)
	{
		using RuleHandler = std::function<bool(Context&)>;
  
		std::map<ParsingRule, const RuleHandler> handlers =
		{
			{ParsingRule::start, parseStart},
			{ParsingRule::command, parseCommand},
			{ParsingRule::sequence_definition, parseSequenceDefinition},
			{ParsingRule::single_cycle, parseSingleCycle},
			{ParsingRule::hush, parseHush},
			{ParsingRule::setCps, parseSetCps},
			{ParsingRule::setBpm, parseSetBpm},
		};

    assert(handlers.size() == size_t(ParsingRule::count));
  
		return handlers[rule](context);
	}

	bool resolveOneOf(Context& context, const std::initializer_list<ParsingRule>& rules)
	{
		const auto it = std::find_if(rules.begin(), rules.end(), [&context](auto r) {
			return resolve(context, r);
		});
		return it != rules.end();
	}
}






