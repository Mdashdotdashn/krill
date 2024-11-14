#include "Rules.hpp"

#include "Context.hpp"
#include "Helpers.hpp"

#include <cassert>
#include <regex>
#include <string>

std::vector<std::string> group(const std::string& input, const std::string& leftDelimiter, const std::string& rightDelimiter)
{
  constexpr auto npos = std::string::npos;

  std::vector<std::string> groups;

  size_t position = 0;
  while (position < input.size())
  {
    size_t left = input.find_first_of(leftDelimiter, position);
    if (left == npos)
    {
      groups.push_back(input.substr(position));
      position = npos;
    }
    else
    {
      if (left > position)
      {
        groups.push_back(input.substr(position, left - position));
      }
      size_t count = 1;
      auto current = left + 1;
      while (count != 0 && current != npos)
      {
        const auto nextLeft = input.find_first_of(leftDelimiter, current);
        const auto nextRight = input.find_first_of(rightDelimiter, current);
        if (nextLeft < nextRight)
        {
          count++;
          current = nextLeft+1;
        }
        else if (nextRight < nextLeft)
        {
          count--;
          current = nextRight+1;
        }
        else
        {
          current = npos;
        }
      }
      if (current != npos)
      {
        const auto spacePosition = input.find_first_of(" ", current);
        current = spacePosition == npos ? input.size() : spacePosition;
      }
      groups.push_back(input.substr(left, current - left));
      position = current+1;
    }
  }
  return groups;
}

// Split a slice into individual elements. Elements can be
//  [] - enclosed groups
//  <> - enclosed groups
//  single elements

std::vector<std::string> splitCycleSlice(const std::string& sliceString)
{

  const auto bracketGroup = group(sliceString, "[", "]");
	
	std::vector<std::string> smallerGreaterGroups;
  for (const auto& g : bracketGroup)
	{
		if (g[0] != '[')
		{
			const auto subGroup = group(g,"<", ">");
			for (const auto& s : subGroup)
			{
				smallerGreaterGroups.push_back(s);
			}
		}
		else
		{
			smallerGreaterGroups.push_back(g);
		}
	}

	std::vector<std::string> allGroups;

  for (const auto& g : smallerGreaterGroups)
	{
		if ((g[0] != '[') && (g[0]!='<'))
		{
      std::istringstream stream(g);
      std::string token;
      std::vector<std::string> tokens;

      while (stream >> token) {
          allGroups.push_back(token);
      }
		}
		else
		{
			allGroups.push_back(g);
		}
	}

  return allGroups;
}

namespace
{
using Context = krill::Context;
using ParsingResult = krill::ParsingResult;
using ParsingException = krill::ParsingException;
using Value = rapidjson::Value;


Value buildXmlForCommand(Context& c, const std::string& command, const std::optional<float> value)
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

Value buildXmlForPattern(Context& c, Value& sources, const std::string& aligment)
{
  using namespace rapidjson;

  assert(sources.IsArray());

  Value result(kObjectType);
  auto& allocator = c.document().GetAllocator();

  // Type
  result.AddMember("type_", "pattern", allocator);
  // Alignment
  Value alignmentString;
  alignmentString.SetString(aligment.c_str(), SizeType(aligment.size()), allocator);
  Value arguments_(kObjectType);
  arguments_.AddMember("alignment", alignmentString, allocator);
  result.AddMember("arguments_", arguments_, allocator);
  // Source
  result.AddMember("source_", sources, allocator);
  return result;
}

rapidjson::Value buildXmlForElement(Context& c, const std::string& source)
{
  using namespace rapidjson;

  Value result(kObjectType);
  auto& allocator = c.document().GetAllocator();

  // Type
  result.AddMember("type_", "element", allocator);

  // Source
  Value sourceString;
  sourceString.SetString(source.c_str(), SizeType(source.size()), allocator);
  result.AddMember("source_", sourceString, allocator);
  return result;
}

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

ParsingResult parseSlice(Context& c, const std::string& content)
{
  using namespace rapidjson;
  Value values(kArrayType);

  // Shortcut: simply tokenize by string for now on
  std::stringstream ss(content);

  std::string token;
  while(std::getline(ss, token, ' '))
  {
    auto xml = buildXmlForElement(c, token);
    values.PushBack(xml, c.document().GetAllocator());
  }
  // Could be contracted if modified in krill.js
  //if (values.Size() > 1)
  //{
  //  return values;
  //}
  //Value firstElement(kObjectType);
  //firstElement = values[0];
  //return firstElement;
  return buildXmlForPattern(c, values, "h");
}

ParsingResult parseSequenceContent(Context& c, const std::string& content)
{
  using namespace rapidjson;
  Value values(kArrayType);

  size_t position = 0;
  while (position != std::string::npos)
  {
    bool success = false;
    auto commaPos = position;
    while (!success && commaPos!= std::string::npos)
    {
      commaPos = content.find_first_of(",", commaPos);
      if (auto result = parseSlice(c, content.substr(position, commaPos - position)))
      {
        values.PushBack(result.value(), c.document().GetAllocator());
        position = commaPos == std::string::npos ? commaPos : commaPos + 1;
        success = true;
      }
    }
    if (!success)
    {
      throw ParsingException();
    }
  }


  if (values.Size() > 1)
  {
    return buildXmlForPattern(c, values,"v");
  }
  Value firstElement(kObjectType);
  firstElement = values[0];
  return firstElement;
}

// Aka mini-notation
ParsingResult parseSequence(Context& c)
{
  if (auto result = c.consumeDelimitedString("\"\'"))
  {
    return parseSequenceContent(c, result.value());
  }
  return {};
}

ParsingResult parseSequenceDefinition(Context& c)
{
  return parseSequence(c);
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

} // namespace

namespace krill
{
// Start point
ParsingResult resolve(Context& c)
{
  return resolveOneOf(c, {
    parseCommand,
    parseSequenceDefinition,
    });
}
}






