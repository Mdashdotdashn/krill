#include "Rules.hpp"
#include "Context.hpp"

namespace
{
	using Context = krill::Context;
	using ParsingRule = krill::ParsingRule;

	bool parseStatement(Context& c)
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
			});
	}

	bool parseSequenceDefinition(Context& c)
	{
		return false;
	}

	bool parseHush(Context& c)
	{
		const std::string token("hush");
		if (c.consumeToken(token))
		{
			c.addCommand(token);
			return true;
		}
		return false;
	}

	bool parseSetBpm(Context& c)
	{
		const std::string token("setBpm");
		if (c.consumeToken(token))
		{
			//		if (const auto n = c.consumeNumber())
			//		{
			//			return CommandStub(token, n.value());			
			//		}
		}
		return false;
	}
}

namespace krill
{
	using RuleHandler = std::function<bool(Context&)>;

	std::map<ParsingRule, RuleHandler> SRuleHandlers()
	{
		return
		{
			{ParsingRule::statement, parseStatement},
			{ParsingRule::command, parseCommand},
			{ParsingRule::sequence_definition, parseSequenceDefinition},
			{ParsingRule::hush, parseHush},
			{ParsingRule::setBpm, parseSetBpm},
		};
	}

	bool resolve(Context& context, ParsingRule rule)
	{
		const auto handler = SRuleHandlers()[rule];
		return handler(context);
	}

	bool resolveOneOf(Context& context, const std::initializer_list<ParsingRule>& rules)
	{
		const auto it = std::find_if(rules.begin(), rules.end(), [&context](auto r) {
			return resolve(context, r);
		});
		return it != rules.end();
	}
}






