#include "RenderTreeBuilder.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

namespace krill
{
namespace detail
{
namespace rj = rapidjson;

RenderNodePtr makeRenderNode(const rj::Value& value);

// Builds an array of RenderNodePtr from a rj array
std::vector<RenderNodePtr> buildStepArray(const rj::Value& stepArray)
{
  assert(stepArray.IsArray());

  std::vector<RenderNodePtr> result;
  for (const auto& element: stepArray.GetArray())
  {
    result.push_back(makeRenderNode(element));
  }
  return result;
}

RenderNodePtr makeOperatorRenderNode(const std::string& type, const rj::Value& arguments, RenderNodePtr childNode)
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

// Factory for a single step within a pattern
RenderNodePtr makeStepRenderNode(const rj::Value& source, const rj::Value& options)
{
  // An element is a single step, it can be either a render node/tree or
  // a single string value
  const auto makeElementSourceNode = [](const rj::Value& s)
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

  // Add weight to the step
  const auto weight = optionOrValue<float>(options, "weight", 1);
  pRenderNode->setWeight(weight);

  return pRenderNode;
}

// Factory for pattern (collection of steps) nodes.
RenderNodePtr makePatternRenderNode(const rj::Value& source, const rj::Value& arguments)
{
  const auto& alignment = arguments["alignment"];
  auto stepArray = buildStepArray(source);

  // Horizontal alignment (regular pattern)
  if (alignment == "h")
  {
    return makeWeightedPatternRenderNode(stepArray);
  }

  // Timeline (sequence of patterns)
  if (alignment == "t")
  {
    return makeTimelineRenderNode(stepArray);
  }

  // Stack multiple sequence playing in parallel
  if (alignment == "v")
  {
    return makeStackRenderNode(stepArray);
  }
  assert(0);
  return nullptr;
}

// Generic render node factory. Returns a RenderNodePtr for the node
// specified by the rj value
RenderNodePtr makeRenderNode(const rj::Value& node)
{
  rj::Value emptyObject;
  emptyObject.SetObject();

  const auto& type = node["type_"];
  const auto& source = node["source_"];
  const auto& options = hasMember(node, "options_") ? node["options_"] : emptyObject;
  const auto& arguments = hasMember(node, "arguments_") ? node["arguments_"] : emptyObject;

  const auto typeString = type.GetString();

  if (type == "pattern")
  {
    return makePatternRenderNode(source, arguments);
  }

  if (type == "element")
  {
    return makeStepRenderNode(source, options);
  }
  // All following are operator and have a single child node
  const auto childNode = makeRenderNode(source);
  return makeOperatorRenderNode(typeString, arguments, childNode);
}
} // namespace detail


RenderNodePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
{
  return detail::makeRenderNode(v);
};
} // namespace kril
