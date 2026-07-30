#include "PatternNodeFactory.hpp"

#include <string>
#include <vector>

#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/TimelinePatternRenderNode.hpp"
#include "renderer/nodes/VerticalPatternRenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeHorizontalPatternNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "pattern")
      {
        return std::nullopt;
      }
      if (!v.HasMember("arguments_") || !v["arguments_"].IsObject())
      {
        return std::nullopt;
      }
      if (!v.HasMember("source_") || !v["source_"].IsArray())
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"];
      if (!args.HasMember("alignment") || !args["alignment"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(args["alignment"].GetString()) != "h")
      {
        return std::nullopt;
      }

      const auto& source = v["source_"].GetArray();
      std::vector<RenderNodePtr> children;
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

      return std::make_shared<HorizontalPatternRenderNode>(children, weights);
    }

    std::optional<RenderNodePtr> makeVerticalPatternNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "pattern")
      {
        return std::nullopt;
      }
      if (!v.HasMember("arguments_") || !v["arguments_"].IsObject())
      {
        return std::nullopt;
      }
      if (!v.HasMember("source_") || !v["source_"].IsArray())
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"];
      if (!args.HasMember("alignment") || !args["alignment"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(args["alignment"].GetString()) != "v")
      {
        return std::nullopt;
      }

      std::vector<RenderNodePtr> children;
      const auto& source = v["source_"].GetArray();
      children.reserve(source.Size());
      for (const auto& child : source)
      {
        children.push_back(buildRenderTree(child));
      }

      return std::make_shared<VerticalPatternRenderNode>(children);
    }

    std::optional<RenderNodePtr> makeTimelinePatternNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&))
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(v["type_"].GetString()) != "pattern")
      {
        return std::nullopt;
      }
      if (!v.HasMember("arguments_") || !v["arguments_"].IsObject())
      {
        return std::nullopt;
      }
      if (!v.HasMember("source_") || !v["source_"].IsArray())
      {
        return std::nullopt;
      }

      const auto& args = v["arguments_"];
      if (!args.HasMember("alignment") || !args["alignment"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(args["alignment"].GetString()) != "t")
      {
        return std::nullopt;
      }

      std::vector<RenderNodePtr> children;
      const auto& source = v["source_"].GetArray();
      children.reserve(source.Size());
      for (const auto& child : source)
      {
        children.push_back(buildRenderTree(child));
      }

      return std::make_shared<TimelinePatternRenderNode>(children);
    }
  }
}
