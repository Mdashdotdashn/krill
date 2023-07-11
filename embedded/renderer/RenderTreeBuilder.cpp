#pragma once

#include "RenderTreeBuilder.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace krill
{
namespace detail
{
RenderNodePtr makeRenderNode(const rapidjson::Value& value);
std::vector<RenderNodePtr> buildStepArray(const rapidjson::Value& a);

RenderNodePtr makeRenderNode(const rapidjson::Value& value)
{
  rapidjson::Value emptyObject;
  emptyObject.SetObject();

  const auto& type = value["type_"];
  const auto& source = value["source_"];
  const auto& options = hasMember(value, "options_") ? value["options_"] : emptyObject;

  const auto typeString = type.GetString();
  if (type == "pattern")
  {
    auto stepArray = buildStepArray(source);
    return makeWeightedPatternRenderNode(stepArray);
  }

  if (type == "element")
  {
    if (source.IsObject())
    {
      return makeRenderNode(source);
    }
    else
    {
      const auto value = source.GetString();
      const auto cycle = makeSingleEventCycle(value);
      return makeStepRenderNode(cycle, options);
    }
  }

  // All following are operator and have a single child node
  const auto childNode = makeRenderNode(source);
  const auto arguments = value["arguments_"].GetArray();

  if (type == "stretch")
  {
    Fraction factor;
    factor.convertDoubleToFraction(arguments[0].GetDouble());
    return makeStretchRenderNode(childNode, factor);
  }

  assert(0);
  return nullptr;
}

std::vector<RenderNodePtr> buildStepArray(const rapidjson::Value& a)
{
  std::vector<RenderNodePtr> result;
  for (rapidjson::SizeType i = 0; i < a.Size(); i++)
  {
    result.push_back(makeRenderNode(a[i]));
  }
  return result;
}
} // namespace detail


RenderNodePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
{
  return detail::makeRenderNode(v);
};
} // namespace kril
