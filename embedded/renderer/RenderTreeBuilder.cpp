#include "RenderTreeBuilder.hpp"

#include <optional>

#include "nodes/ElementRenderNode.hpp"
#include "nodes/EmptyRenderNode.hpp"
#include "nodes/HorizontalPatternRenderNode.hpp"
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

    std::optional<std::vector<RenderTreePtr>> horizontalPatternChildren(const rapidjson::Value& v)
    {
      if (!isHorizontalPatternNode(v))
      {
        return std::nullopt;
      }

      const auto& source = v["source_"].GetArray();
      if (source.Empty())
      {
        return std::vector<RenderTreePtr>{};
      }

      std::vector<RenderTreePtr> children;
      children.reserve(source.Size());
      for (const auto& child : source)
      {
        children.push_back(buildRenderTree(child));
      }

      return children;
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

    RenderTreePtr buildRenderTree(const rapidjson::Value& v)
    {
      if (isElementNode(v))
      {
        const auto& source = v["source_"];
        if (source.IsObject())
        {
          return std::make_shared<ElementRenderNode>(buildRenderTree(source));
        }
        return std::make_shared<ElementRenderNode>(sourceAsString(source));
      }

      const auto horizontalChildren = horizontalPatternChildren(v);
      if (horizontalChildren.has_value())
      {
        return std::make_shared<HorizontalPatternRenderNode>(horizontalChildren.value());
      }

      const auto verticalChildren = verticalPatternChildren(v);
      if (verticalChildren.has_value())
      {
        return std::make_shared<VerticalPatternRenderNode>(verticalChildren.value());
      }

      return std::make_shared<EmptyRenderNode>();
    }
  }

  RenderTreePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
  {
    return buildRenderTree(v);
  }
}
