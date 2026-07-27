#include "StructNodeFactory.hpp"

#include <string>

#include "renderer/nodes/StructRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeStructNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "struct")
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

      return std::make_shared<StructRenderNode>(buildRenderTree(args[0]), buildRenderTree(v["source_"]));
    }
  }
}
