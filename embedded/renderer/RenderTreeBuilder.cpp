#include "RenderTreeBuilder.hpp"

#include "utils/jsonUtils.hpp"

#include <cassert>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace krill
{
namespace detail
{
namespace rj = rapidjson;

namespace detail
{
  Fraction fractionFromValue(const rj::Value& v)
  {
    Fraction factor;
    if (v.IsNumber())
    {
      factor.convertDoubleToFraction(v.GetDouble());
    }
    else
    {
      factor.convertStringToFraction(v.GetString());
    }
    return factor;
  }

  int intFromValue(const rj::Value& v)
  {
    if (v.IsInt())
    {
      return v.GetInt();
    }

    if (v.IsNumber())
    {
      return static_cast<int>(v.GetDouble());
    }

    return std::atoi(v.GetString());
  }

  std::vector<int> bjorklundBits(int steps, int pulses)
  {
    steps = std::abs(steps);
    pulses = std::abs(pulses);

    if (pulses > steps || pulses == 0 || steps == 0)
    {
      return {};
    }

    std::vector<int> pattern;
    std::vector<int> counts;
    std::vector<int> remainders;

    int divisor = steps - pulses;
    remainders.push_back(pulses);
    int level = 0;

    while (true)
    {
      counts.push_back(divisor / remainders[level]);
      remainders.push_back(divisor % remainders[level]);
      divisor = remainders[level];
      level += 1;
      if (remainders[level] <= 1)
      {
        break;
      }
    }

    counts.push_back(divisor);

    std::function<void(int)> build = [&](int lvl) {
      if (lvl > -1)
      {
        for (int i = 0; i < counts[lvl]; i++)
        {
          build(lvl - 1);
        }
        if (remainders[lvl] != 0)
        {
          build(lvl - 2);
        }
      }
      else if (lvl == -1)
      {
        pattern.push_back(0);
      }
      else if (lvl == -2)
      {
        pattern.push_back(1);
      }
    };

    build(level);
    std::reverse(pattern.begin(), pattern.end());
    return pattern;
  }

  struct BjorklundGroup
  {
    int value{0};
    int weight{1};
  };

  std::vector<BjorklundGroup> bjorklundGroups(const std::vector<int>& bits)
  {
    std::vector<BjorklundGroup> result;
    if (bits.empty())
    {
      return result;
    }

    result.push_back({bits.front(), 1});
    for (size_t i = 1; i < bits.size(); i++)
    {
      const auto bit = bits[i];
      if (bit == 0)
      {
        result.back().weight += 1;
      }
      else
      {
        result.push_back({1, 1});
      }
    }
    return result;
  }
} // namespace detail

RenderNodePtr makeRenderNode(const rj::Value& value);

RenderNodePtr makeBjorklundRenderNode(const rj::Value& sourceNode, int pulses, int steps)
{
  const auto bits = detail::bjorklundBits(steps, pulses);
  const auto groups = detail::bjorklundGroups(bits);

  // Keep behavior deterministic for invalid inputs.
  if (groups.empty())
  {
    return makeCycleRenderNode(makeSingleEventCycle("~"));
  }

  std::vector<RenderNodePtr> weightedNodes;
  weightedNodes.reserve(groups.size());

  for (const auto& g : groups)
  {
    RenderNodePtr node;
    if (g.value == 0)
    {
      node = makeCycleRenderNode(makeSingleEventCycle("~"));
    }
    else
    {
      // Build a fresh subtree per pulse group to avoid shared tick state.
      node = makeRenderNode(sourceNode);
    }

    node->setWeight(static_cast<float>(g.weight));
    weightedNodes.push_back(node);
  }

  return makeWeightedPatternRenderNode(weightedNodes);
}

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

// Build a render node for operator arguments.
// Object arguments are normalized to a repeating one-cycle view before
// downstream sampling/weaving operators consume them.
RenderNodePtr buildRenderNodeForArgument(const rj::Value& argument)
{
  if (argument.IsObject())
  {
    return std::make_shared<NormalizeCycleRenderNode>(makeRenderNode(argument));
  }

  return makeRenderNode(argument);
}

RenderNodePtr makeOperatorRenderNode(const std::string& type,
                                     const rj::Value& arguments,
                                     RenderNodePtr childNode,
                                     const rj::Value* sourceJson = nullptr)
{
  assert(arguments.IsArray());

  if (type == "stretch")
  {
    // Parser-level slow/fast and slice /,* canonicalize to stretch.
    Fraction factor = detail::fractionFromValue(arguments[0]);
    return makeStretchRenderNode(childNode, factor);
  }

  if (type == "fixed-step")
  {
    // Parser-level % slice modifier canonicalizes to fixed-step.
    Fraction stepDivision = detail::fractionFromValue(arguments[0]);
    return makeFixedStepRenderNode(childNode, stepDivision);
  }

  if (type == "trunc")
  {
    Fraction length = detail::fractionFromValue(arguments[0]);
    return makeTruncRenderNode(childNode, length);
  }

  if (type == "shift")
  {
    if (arguments[0].IsObject())
    {
      const auto shiftNode = buildRenderNodeForArgument(arguments[0]);
      const auto direction = arguments.Size() > 1
                               ? detail::fractionFromValue(arguments[1])
                               : Fraction(1);
      return makeShiftRenderNode(childNode, shiftNode, direction);
    }

    Fraction offset = detail::fractionFromValue(arguments[0]);
    return makeShiftRenderNode(childNode, offset);
  }

  if (type == "struct")
  {
    assert(arguments.Size() >= 1);
    // arguments[0] is the struct pattern (right operand)
    // childNode is the left operand
    const auto rightNode = buildRenderNodeForArgument(arguments[0]);
    return makeStructRenderNode(childNode, rightNode);
  }

  if (type == "add")
  {
    assert(arguments.Size() >= 1);
    // arguments[0] is the add pattern (right operand)
    // childNode is the left operand
    const auto rightNode = buildRenderNodeForArgument(arguments[0]);
    return makeAddRenderNode(childNode, rightNode);
  }

  if (type == "scale")
  {
    assert(arguments.Size() >= 1);
    assert(arguments[0].IsString());
    return makeScaleRenderNode(childNode, arguments[0].GetString());
  }

  if (type == "bjorklund")
  {
    assert(arguments.Size() >= 2);
    assert(sourceJson != nullptr);
    const int pulses = detail::intFromValue(arguments[0]);
    const int steps = detail::intFromValue(arguments[1]);
    return makeBjorklundRenderNode(*sourceJson, pulses, steps);
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
    pRenderNode = makeOperatorRenderNode(type, arguments, pRenderNode, &source);
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
  return makeOperatorRenderNode(typeString, arguments, childNode, &source);
}
} // namespace detail


RenderNodePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
{
  return detail::makeRenderNode(v);
};
} // namespace kril
