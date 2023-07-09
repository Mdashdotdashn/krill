#pragma once

#include "RenderNode.hpp"

#include <rapidjson/document.h>

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
  const auto& options = value["options_"];

  const auto typeString = type.GetString();
  if (type == "pattern")
  {
    const auto stepArray = buildStepArray(source);
//    return
  }

  if (type == "element")
  {
    const auto value = source.GetString();
    const auto cycle = makeSingleEventCycle(value);
    return makeStepRenderNode(cycle, options);
  }
  return std::make_shared<RenderNode>();
}

RenderNodePtr makeRenderNode(const rapidjson::Value& value)
{
  if (value.IsObject())
  {
      return makeOperatorRenderNode(value);
  }
  return std::make_shared<RenderNode>();
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


class RenderTreeBuilder
{
public:
  static RenderNodePtr fromJson(const rapidjson::Document& document)
  {
    return detail::makeRenderNode(document);
  }
};
} // namespace kril
