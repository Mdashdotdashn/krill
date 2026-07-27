#include "BjorklundNodeFactory.hpp"

#include "renderer/nodes/BjorklundRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeBjorklundNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<long> (*valueAsLong)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "bjorklund")
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

      const auto pulses = valueAsLong(args[0]);
      const auto steps = valueAsLong(args[1]);
      if (!pulses.has_value() || !steps.has_value())
      {
        return std::nullopt;
      }

      return std::make_shared<BjorklundRenderNode>(buildRenderTree(v["source_"]), pulses.value(), steps.value());
    }
  }
}
