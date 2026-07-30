#include "ShiftNodeFactory.hpp"

#include <string>

#include "../nodes/ShiftRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeShiftNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&),
      std::optional<long> (*valueAsLong)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "shift")
      {
        return std::nullopt;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"].GetArray();
      if (args.Size() < 2)
      {
        return std::nullopt;
      }

      const auto maybeDirection = valueAsLong(args[1]);
      if (!maybeDirection.has_value())
      {
        return std::nullopt;
      }

      const auto source = buildRenderTree(v["source_"]);

      if (args[0].IsObject())
      {
        return std::make_shared<ShiftRenderNode>(source, buildRenderTree(args[0]), maybeDirection.value());
      }

      const auto maybeAmount = valueAsFraction(args[0]);
      if (!maybeAmount.has_value())
      {
        return std::nullopt;
      }

      return std::make_shared<ShiftRenderNode>(source, maybeAmount.value(), maybeDirection.value());
    }
  }
}
