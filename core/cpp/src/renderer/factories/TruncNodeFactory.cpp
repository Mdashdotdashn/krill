#include "TruncNodeFactory.hpp"

#include <string>

#include "../nodes/TruncRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeTruncNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "trunc")
      {
        return std::nullopt;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"].GetArray();
      if (args.Size() == 0)
      {
        return std::nullopt;
      }

      const auto maybeLength = valueAsFraction(args[0]);
      if (!maybeLength.has_value())
      {
        return std::nullopt;
      }

      return std::make_shared<TruncRenderNode>(buildRenderTree(v["source_"]), maybeLength.value());
    }
  }
}
