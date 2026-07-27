#include "ElementNodeFactory.hpp"

#include <string>

#include "renderer/nodes/BjorklundRenderNode.hpp"
#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/StretchRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    namespace
    {
      Fraction sourceUnitsForFixedStep(
        const rapidjson::Value& source,
        std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&))
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
            return sourceUnitsForFixedStep(source["source_"], valueAsFraction);
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

      RenderNodePtr applyElementOperator(
        RenderNodePtr node,
        const rapidjson::Value& elementNode,
        std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&),
        std::optional<long> (*valueAsLong)(const rapidjson::Value&))
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
            const auto units = sourceUnitsForFixedStep(elementNode["source_"], valueAsFraction);
            return std::make_shared<StretchRenderNode>(node, units / stepSize.value());
          }
        }

        return node;
      }
    }

    std::optional<RenderNodePtr> makeElementNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::string (*sourceAsString)(const rapidjson::Value&),
      std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&),
      std::optional<long> (*valueAsLong)(const rapidjson::Value&))
    {
      if (!v.IsObject())
      {
        return std::nullopt;
      }
      if (!v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "element")
      {
        return std::nullopt;
      }
      if (!v.HasMember("source_"))
      {
        return std::nullopt;
      }

      const auto& source = v["source_"];
      if (source.IsObject())
      {
        auto element = std::make_shared<ElementRenderNode>(buildRenderTree(source));
        return applyElementOperator(element, v, valueAsFraction, valueAsLong);
      }

      auto element = std::make_shared<ElementRenderNode>(sourceAsString(source));
      return applyElementOperator(element, v, valueAsFraction, valueAsLong);
    }
  }
}
