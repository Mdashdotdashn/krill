#include "RenderTreeBuilder.hpp"

#include <optional>

#include "nodes/AddRenderNode.hpp"
#include "nodes/BjorklundRenderNode.hpp"
#include "nodes/ElementRenderNode.hpp"
#include "nodes/EmptyRenderNode.hpp"
#include "nodes/HorizontalPatternRenderNode.hpp"
#include "nodes/StretchRenderNode.hpp"
#include "nodes/StructRenderNode.hpp"
#include "nodes/TimelinePatternRenderNode.hpp"
#include "nodes/VerticalPatternRenderNode.hpp"

namespace krill
{
  namespace
  {
    RenderTreePtr buildRenderTree(const rapidjson::Value& v);

    bool isElementNode(const rapidjson::Value& v)
    {
      if (!v.IsObject())
      {
        return false;
      }
      if (!v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      return std::string(v["type_"].GetString()) == "element" && v.HasMember("source_");
    }

    std::string sourceAsString(const rapidjson::Value& source)
    {
      if (source.IsString())
      {
        return source.GetString();
      }
      if (source.IsBool())
      {
        return source.GetBool() ? "true" : "false";
      }
      if (source.IsInt())
      {
        return std::to_string(source.GetInt());
      }
      if (source.IsInt64())
      {
        return std::to_string(source.GetInt64());
      }
      if (source.IsUint())
      {
        return std::to_string(source.GetUint());
      }
      if (source.IsUint64())
      {
        return std::to_string(source.GetUint64());
      }
      if (source.IsDouble())
      {
        return std::to_string(source.GetDouble());
      }
      return "";
    }

    std::optional<Fraction> valueAsFraction(const rapidjson::Value& v)
    {
      if (v.IsInt())
      {
        return Fraction(static_cast<long>(v.GetInt()), 1);
      }
      if (v.IsInt64())
      {
        return Fraction(static_cast<long>(v.GetInt64()), 1);
      }
      if (v.IsUint())
      {
        return Fraction(static_cast<long>(v.GetUint()), 1);
      }
      if (v.IsUint64())
      {
        return Fraction(static_cast<long>(v.GetUint64()), 1);
      }
      if (v.IsDouble())
      {
        return Fraction(v.GetDouble());
      }
      if (v.IsString())
      {
        return Fraction(std::string(v.GetString()));
      }
      return std::nullopt;
    }

    Fraction sourceUnitsForFixedStep(const rapidjson::Value& source)
    {
      if (!source.IsObject())
      {
        return Fraction(1);
      }

      if (source.HasMember("type_") && source["type_"].IsString())
      {
        const auto type = std::string(source["type_"].GetString());

        if (type == "element" && source.HasMember("source_"))
        {
          return sourceUnitsForFixedStep(source["source_"]);
        }

        if (type == "pattern"
            && source.HasMember("arguments_")
            && source["arguments_"].IsObject()
            && source["arguments_"].HasMember("alignment")
            && source["arguments_"]["alignment"].IsString()
            && std::string(source["arguments_"]["alignment"].GetString()) == "h"
            && source.HasMember("source_")
            && source["source_"].IsArray())
        {
          Fraction total(0);
          for (const auto& child : source["source_"].GetArray())
          {
            Fraction weight(1);
            if (child.IsObject() && child.HasMember("options_") && child["options_"].IsObject())
            {
              const auto& options = child["options_"];
              if (options.HasMember("weight"))
              {
                const auto maybeWeight = valueAsFraction(options["weight"]);
                if (maybeWeight.has_value() && maybeWeight.value() > Fraction(0))
                {
                  weight = maybeWeight.value();
                }
              }
            }
            total += weight;
          }
          if (total > Fraction(0))
          {
            return total;
          }
        }
      }

      return Fraction(1);
    }

    std::optional<long> valueAsLong(const rapidjson::Value& v)
    {
      if (v.IsInt()) return static_cast<long>(v.GetInt());
      if (v.IsInt64()) return static_cast<long>(v.GetInt64());
      if (v.IsUint()) return static_cast<long>(v.GetUint());
      if (v.IsUint64()) return static_cast<long>(v.GetUint64());
      if (v.IsDouble()) return static_cast<long>(v.GetDouble());
      if (v.IsString())
      {
        try
        {
          return std::stol(std::string(v.GetString()));
        }
        catch (...)
        {
          return std::nullopt;
        }
      }
      return std::nullopt;
    }

    RenderTreePtr applyElementOperator(RenderTreePtr node, const rapidjson::Value& elementNode)
    {
      if (!elementNode.IsObject() || !elementNode.HasMember("options_") || !elementNode["options_"].IsObject())
      {
        return node;
      }

      const auto& options = elementNode["options_"];
      if (!options.HasMember("operator") || !options["operator"].IsObject())
      {
        return node;
      }

      const auto& op = options["operator"];
      if (!op.HasMember("type_") || !op["type_"].IsString())
      {
        return node;
      }
      if (!op.HasMember("arguments_") || !op["arguments_"].IsArray())
      {
        return node;
      }

      const auto opType = std::string(op["type_"].GetString());
      const auto& args = op["arguments_"].GetArray();

      if (opType == "bjorklund" && args.Size() >= 2)
      {
        const auto pulses = valueAsLong(args[0]);
        const auto steps = valueAsLong(args[1]);
        if (pulses.has_value() && steps.has_value())
        {
          return std::make_shared<BjorklundRenderNode>(node, pulses.value(), steps.value());
        }
      }

      if (opType == "stretch" && args.Size() > 0)
      {
        const auto factor = valueAsFraction(args[0]);
        if (factor.has_value())
        {
          return std::make_shared<StretchRenderNode>(node, factor.value());
        }
      }

      if (opType == "fixed-step" && args.Size() > 0)
      {
        const auto stepSize = valueAsFraction(args[0]);
        if (stepSize.has_value() && stepSize.value() > Fraction(0))
        {
          const auto units = sourceUnitsForFixedStep(elementNode["source_"]);
          return std::make_shared<StretchRenderNode>(node, units / stepSize.value());
        }
      }

      return node;
    }

    bool isHorizontalPatternNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "pattern")
      {
        return false;
      }
      if (!v.HasMember("arguments_") || !v["arguments_"].IsObject())
      {
        return false;
      }
      const auto& args = v["arguments_"];
      if (!args.HasMember("alignment") || !args["alignment"].IsString())
      {
        return false;
      }
      if (std::string(args["alignment"].GetString()) != "h")
      {
        return false;
      }
      return v.HasMember("source_") && v["source_"].IsArray();
    }

    std::optional<std::pair<std::vector<RenderTreePtr>, std::vector<Fraction>>> horizontalPatternChildrenAndWeights(const rapidjson::Value& v)
    {
      if (!isHorizontalPatternNode(v))
      {
        return std::nullopt;
      }

      const auto& source = v["source_"].GetArray();
      if (source.Empty())
      {
        return std::make_pair(std::vector<RenderTreePtr>{}, std::vector<Fraction>{});
      }

      std::vector<RenderTreePtr> children;
      std::vector<Fraction> weights;
      children.reserve(source.Size());
      weights.reserve(source.Size());
      for (const auto& child : source)
      {
        children.push_back(buildRenderTree(child));

        Fraction weight(1);
        if (child.IsObject() && child.HasMember("options_") && child["options_"].IsObject())
        {
          const auto& options = child["options_"];
          if (options.HasMember("weight"))
          {
            const auto maybeWeight = valueAsFraction(options["weight"]);
            if (maybeWeight.has_value() && maybeWeight.value() > Fraction(0))
            {
              weight = maybeWeight.value();
            }
          }
        }
        weights.push_back(weight);
      }

      return std::make_pair(children, weights);
    }

    bool isVerticalPatternNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "pattern")
      {
        return false;
      }
      if (!v.HasMember("arguments_") || !v["arguments_"].IsObject())
      {
        return false;
      }
      const auto& args = v["arguments_"];
      if (!args.HasMember("alignment") || !args["alignment"].IsString())
      {
        return false;
      }
      if (std::string(args["alignment"].GetString()) != "v")
      {
        return false;
      }
      return v.HasMember("source_") && v["source_"].IsArray();
    }

    bool isTimelinePatternNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "pattern")
      {
        return false;
      }
      if (!v.HasMember("arguments_") || !v["arguments_"].IsObject())
      {
        return false;
      }
      const auto& args = v["arguments_"];
      if (!args.HasMember("alignment") || !args["alignment"].IsString())
      {
        return false;
      }
      if (std::string(args["alignment"].GetString()) != "t")
      {
        return false;
      }
      return v.HasMember("source_") && v["source_"].IsArray();
    }

    std::optional<std::vector<RenderTreePtr>> verticalPatternChildren(const rapidjson::Value& v)
    {
      if (!isVerticalPatternNode(v))
      {
        return std::nullopt;
      }

      std::vector<RenderTreePtr> children;
      const auto& source = v["source_"].GetArray();
      children.reserve(source.Size());
      for (const auto& child : source)
      {
        children.push_back(buildRenderTree(child));
      }

      return children;
    }

    std::optional<std::vector<RenderTreePtr>> timelinePatternChildren(const rapidjson::Value& v)
    {
      if (!isTimelinePatternNode(v))
      {
        return std::nullopt;
      }

      std::vector<RenderTreePtr> children;
      const auto& source = v["source_"].GetArray();
      children.reserve(source.Size());
      for (const auto& child : source)
      {
        children.push_back(buildRenderTree(child));
      }

      return children;
    }

    bool isStretchNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "stretch")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() > 0;
    }

    bool isBjorklundNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "bjorklund")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() >= 2;
    }

    bool isStructNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "struct")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() > 0;
    }

    bool isAddNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "add")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() > 0;
    }

    std::optional<std::pair<RenderTreePtr, Fraction>> stretchSourceAndFactor(const rapidjson::Value& v)
    {
      if (!isStretchNode(v))
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"].GetArray();
      const auto maybeFactor = valueAsFraction(args[0]);
      if (!maybeFactor.has_value())
      {
        return std::nullopt;
      }

      return std::make_pair(buildRenderTree(v["source_"]), maybeFactor.value());
    }

    std::optional<std::tuple<RenderTreePtr, long, long>> bjorklundSourceAndParams(const rapidjson::Value& v)
    {
      if (!isBjorklundNode(v))
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"].GetArray();
      const auto pulses = valueAsLong(args[0]);
      const auto steps = valueAsLong(args[1]);
      if (!pulses.has_value() || !steps.has_value())
      {
        return std::nullopt;
      }

      return std::make_tuple(buildRenderTree(v["source_"]), pulses.value(), steps.value());
    }

    std::optional<std::pair<RenderTreePtr, RenderTreePtr>> structMaskAndSource(const rapidjson::Value& v)
    {
      if (!isStructNode(v))
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"].GetArray();
      return std::make_pair(buildRenderTree(args[0]), buildRenderTree(v["source_"]));
    }

    std::optional<std::pair<RenderTreePtr, RenderTreePtr>> addOperands(const rapidjson::Value& v)
    {
      if (!isAddNode(v))
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"].GetArray();
      RenderTreePtr lhs;
      if (args[0].IsObject())
      {
        lhs = buildRenderTree(args[0]);
      }
      else
      {
        lhs = std::make_shared<ElementRenderNode>(sourceAsString(args[0]));
      }

      auto rhs = buildRenderTree(v["source_"]);
      return std::make_pair(lhs, rhs);
    }

    RenderTreePtr buildRenderTree(const rapidjson::Value& v)
    {
      if (isElementNode(v))
      {
        const auto& source = v["source_"];
        if (source.IsObject())
        {
          auto element = std::make_shared<ElementRenderNode>(buildRenderTree(source));
          return applyElementOperator(element, v);
        }
        auto element = std::make_shared<ElementRenderNode>(sourceAsString(source));
        return applyElementOperator(element, v);
      }

      const auto horizontalChildrenAndWeights = horizontalPatternChildrenAndWeights(v);
      if (horizontalChildrenAndWeights.has_value())
      {
        return std::make_shared<HorizontalPatternRenderNode>(horizontalChildrenAndWeights->first, horizontalChildrenAndWeights->second);
      }

      const auto verticalChildren = verticalPatternChildren(v);
      if (verticalChildren.has_value())
      {
        return std::make_shared<VerticalPatternRenderNode>(verticalChildren.value());
      }

      const auto timelineChildren = timelinePatternChildren(v);
      if (timelineChildren.has_value())
      {
        return std::make_shared<TimelinePatternRenderNode>(timelineChildren.value());
      }

      const auto stretch = stretchSourceAndFactor(v);
      if (stretch.has_value())
      {
        return std::make_shared<StretchRenderNode>(stretch->first, stretch->second);
      }

      const auto bjorklund = bjorklundSourceAndParams(v);
      if (bjorklund.has_value())
      {
        return std::make_shared<BjorklundRenderNode>(
          std::get<0>(bjorklund.value()),
          std::get<1>(bjorklund.value()),
          std::get<2>(bjorklund.value()));
      }

      const auto structData = structMaskAndSource(v);
      if (structData.has_value())
      {
        return std::make_shared<StructRenderNode>(structData->first, structData->second);
      }

      const auto addData = addOperands(v);
      if (addData.has_value())
      {
        return std::make_shared<AddRenderNode>(addData->first, addData->second);
      }

      return std::make_shared<EmptyRenderNode>();
    }
  }

  RenderTreePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
  {
    return buildRenderTree(v);
  }
}
