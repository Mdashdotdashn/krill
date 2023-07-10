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

RenderNodePtr makeOperatorRenderNode(const rapidjson::Value& value)
{
  const auto& type = value["type_"];
  const auto& source = value["source_"];

  Options options;
  if (hasMember(value, "options"))
  {
    for (auto& m : value["options"].GetObject())
    {
      options.insert({ std::string(m.name.GetString()), std::string(m.value.GetString()) });
    }
  }

  const auto typeString = type.GetString();
  if (type == "pattern")
  {
    auto stepArray = buildStepArray(source);
    return makeWeightedPatternRenderNode(stepArray);
  }

  if (type == "element")
  {
    const auto value = source.GetString();
    const auto cycle = makeSingleEventCycle(value);
    return makeStepRenderNode(cycle, options);
  }
  return nullptr;
}

RenderNodePtr makeRenderNode(const rapidjson::Value& value)
{
  if (value.IsObject())
  {
      return makeOperatorRenderNode(value);
  }
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
