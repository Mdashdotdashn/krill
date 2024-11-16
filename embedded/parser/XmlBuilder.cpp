#include "XmlBuilder.hpp"

#include "Context.hpp"
#include "Types.hpp"

namespace krill
{
rapidjson::Value buildXmlForCommand(Context& c, const std::string& command, const std::optional<float> value)
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

rapidjson::Value buildXmlForPattern(Context& c, rapidjson::Value& sources, const std::string& aligment)
{
  using namespace rapidjson;

  assert(sources.IsArray());

  if (sources.Size() == 1)
  {
    Value firstElement(kObjectType);
    firstElement = sources[0];
    return firstElement;
  }

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
}