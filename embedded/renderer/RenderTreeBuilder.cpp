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

RenderNodePtr makeOperatorRenderNode(const std::string& type, const rapidjson::Value& arguments, RenderNodePtr childNode)
{
  assert(arguments.IsArray());

  if (type == "stretch")
  {
    Fraction factor;
    if (arguments[0].IsNumber())
    {
      factor.convertDoubleToFraction(arguments[0].GetDouble());
    }
    else
    {
      factor.convertStringToFraction(arguments[0].GetString());
    }
    return makeStretchRenderNode(childNode, factor);
  }

  assert(0);
  return nullptr;

}
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
    // An element is a single step, it can be either a render node/tree or
    // a single string value
    const auto makeElementSourceNode = [](const rapidjson::Value& s)
    {
      if (s.IsObject())
      {
        return makeRenderNode(s);
      }

      assert(s.IsString());
      const auto value = s.GetString();
      const auto cycle = makeSingleEventCycle(value);
      return makeCycleRenderNode(cycle);
    };

    // Build the source node
    auto pRenderNode = makeElementSourceNode(source);

    // Add an additional operator if required
    if (hasMember(options, "operator"))
    {
      const auto op = options["operator"].GetObject();
      const auto type = op["type_"].GetString();
      const auto arguments = op["arguments_"].GetArray();
      pRenderNode = makeOperatorRenderNode(type, arguments, pRenderNode);
    }

    // Wrap it with a slicer so every step
    // has exactly one cycle when rendered
    pRenderNode = std::make_shared<SliceRenderNode>(pRenderNode);

    // Add weight when needed
    const auto weight = optionOrValue<float>(options, "weight", 1);
    pRenderNode->setWeight(weight);

    return pRenderNode;
  }

  // All following are operator and have a single child node
  const auto childNode = makeRenderNode(source);
  const auto arguments = value["arguments_"].GetArray();
  return makeOperatorRenderNode(typeString, arguments, childNode);
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
