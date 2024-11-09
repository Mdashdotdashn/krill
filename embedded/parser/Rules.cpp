#include "Rules.hpp"
#include "Context.hpp"

#include <cassert>
#include <string>

namespace
{
	using Context = krill::Context;
  using ParsingResult = krill::ParsingResult;
	using ParsingException = krill::ParsingException;
  using Value = rapidjson::Value;

rapidjson::Value makeCommand(Context& c, const std::string& command, const std::optional<float> value)
{
	using namespace rapidjson;

  Value result(kObjectType);
  auto& allocator = c.document().GetAllocator();
	// Commands are top level so we can add directly to the document
	result.AddMember("type_", "command", allocator);
	Value name_;
	name_.SetString(command.c_str(), rapidjson::SizeType(command.size()), allocator);
	result.AddMember("name_", name_, allocator);
	if (value)
	{
		const float floatValue = value.value();
		Value value_;
		value_.SetFloat(floatValue);
		Value options_(kObjectType);
		options_.AddMember("value", value_, allocator);
		result.AddMember("options_", options_, allocator);
	}
  return result;
}


  enum class ParsingRule
  {
    command,
    sequence_definition,
    hush,
    setCps,
    setBpm,
    single_cycle,
    count
  };

	ParsingResult resolveOneOf(Context& context, const std::initializer_list<ParsingRule>& rules);
	ParsingResult resolve(Context& context, ParsingRule rule);

	ParsingResult parseCommand(Context& c)
	{
		return resolveOneOf(c, {
			ParsingRule::hush,
			ParsingRule::setBpm,
			ParsingRule::setCps,
			});
	}

	ParsingResult parseSequenceDefinition(Context& c)
	{
		return resolve(c, ParsingRule::single_cycle);
	}

	ParsingResult parseSingleCycle(Context& c)
	{
    return {};
	}

	ParsingResult parseHush(Context& c)
	{
		const std::string token("hush");
		if (c.consumeToken(token))
		{
			return makeCommand(c, token, {});
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
					return makeCommand(c, token, f);
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
					return makeCommand(c, "setcps", f/120.f/2.f);
				}
				throw ParsingException();
		}
		return {};
	}

	ParsingResult resolve(Context& context, ParsingRule rule)
	{
		using RuleHandler = std::function<ParsingResult(Context&)>;
  
		std::map<ParsingRule, const RuleHandler> handlers =
		{
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

	ParsingResult resolveOneOf(Context& context, const std::initializer_list<ParsingRule>& rules)
	{
    for (auto& rule: rules)
    {
      if (auto result = resolve(context, rule))
      {
        return result;
      }
    }
    return {};
	}
} // namespace

namespace krill
{
// Start point
	ParsingResult resolve(Context& c)
	{
		return resolveOneOf(c, {
			ParsingRule::command,
			ParsingRule::sequence_definition,
			});
	}
}






