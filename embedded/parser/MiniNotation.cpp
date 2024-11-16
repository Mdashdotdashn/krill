#include "MiniNotation.hpp"
#include "XmlBuilder.hpp"

#include <functional>
#include <string>

namespace krill
{
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
      current = std::min(current, input.size());
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


using Context = krill::Context;
using ParsingResult = krill::ParsingResult;
using ParsingException = krill::ParsingException;
using Value = rapidjson::Value;


Value parseSlice(Context& context, const std::string& content);

Value parseSubCycle(Context& context, const std::string& content)
{
  constexpr auto npos = std::string::npos;
  // Split in groups first and then process the elements that aren't grouped by [] pr <> to find the commas in them.

  std::vector<std::string> slices;
  std::string current;

  const auto groups = splitCycleSlice(content);
  for (const auto& group : groups)
  {
    if ((group[0] != '[') && (group[1] != '<'))
    {
      const auto  commaPos = group.find_first_of(',');
      if (commaPos != npos)
      {
        current += " " + group.substr(0, commaPos);
        slices.push_back(current);
        current = group.substr(commaPos+1);
      }
      else
      {
        current += " " + group;
      }
    }
      else
      {
        current += " " + group;
      }
  }
  if (current.size() > 0)
  {
    slices.push_back(current);
  }

  using namespace rapidjson;
  Value values(kArrayType);

  for (const auto& slice : slices)
  {
    values.PushBack(parseSlice(context, slice), context.document().GetAllocator());
  }

  if (values.Size() > 1)
  {
    return buildXmlForPattern(context, values,"v");
  }
  Value firstElement(kObjectType);
  firstElement = values[0];
  return firstElement;
}

Value parseSliceElement(Context& context, const std::string& content)
{
  const auto first = content.front();
  const auto last = content.back();
  if (first == '[')
  {
    assert(last == ']');
    return parseSubCycle(context, content.substr(1, content.size() -2));
  }
  else if (first == '<')
  {
    assert(last == '>');
    assert(0);
//    return parseTimeLine(context, content)
  }
  return buildXmlForElement(context, content);
}

Value parseSlice(Context& context, const std::string& content)
{
  using namespace rapidjson;
  Value values(kArrayType);

  auto& allocator = context.document().GetAllocator();

  const auto elements = splitCycleSlice(content);
  for (const auto& element : elements)
  {
    auto value = parseSliceElement(context, element);
    values.PushBack(value, allocator);
  }

  return buildXmlForPattern(context, values, "h");
}

// Parse the sequence by splitting it first into comma - separated slices
// Then parse every slice
Value parseSequenceContent(Context& c, const std::string& content)
{
  return parseSubCycle(c, content);
}

// Aka mini-notation. Expect quotes and parse its content
ParsingResult parseMiniNotation(Context& c)
{
  if (auto result = c.consumeDelimitedString("\"\'"))
  {
    return parseSequenceContent(c, result.value());
  }
  return {};
}
} // namespace krill
