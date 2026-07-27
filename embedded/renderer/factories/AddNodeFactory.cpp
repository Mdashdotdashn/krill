#include "AddNodeFactory.hpp"

#include <string>

#include "renderer/nodes/AddRenderNode.hpp"
#include "renderer/nodes/ElementRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeAddNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::string (*sourceAsString)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "add")
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

      RenderNodePtr lhs;
      if (args[0].IsObject())
      {
        lhs = buildRenderTree(args[0]);
      }
      else
      {
        lhs = std::make_shared<ElementRenderNode>(sourceAsString(args[0]));
      }

      auto rhs = buildRenderTree(v["source_"]);
      return std::make_shared<AddRenderNode>(lhs, rhs);
    }
  }
}
