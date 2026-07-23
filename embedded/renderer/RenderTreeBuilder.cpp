#include "RenderTreeBuilder.hpp"

#include <optional>

namespace krill
{
  namespace
  {
    class EmptyRenderTree final : public RenderTree
    {
    public:
      std::vector<QueryFragment> query(const QueryRequest& request) const override
      {
        (void)request;
        return {};
      }
    };

    class SingleEventRenderTree final : public RenderTree
    {
    public:
      explicit SingleEventRenderTree(std::string value)
      : mValue(std::move(value))
      {
      }

      std::vector<QueryFragment> query(const QueryRequest& request) const override
      {
        const auto requestStart = request.start;
        const auto requestEnd = request.end;
        if (requestStart == requestEnd)
        {
          return {};
        }

        const Fraction wholeStart(0);
        const Fraction wholeEnd(1);
        if (requestEnd <= wholeStart || requestStart >= wholeEnd)
        {
          return {};
        }

        QueryFragment fragment;
        fragment.wholeStart = wholeStart;
        fragment.wholeEnd = wholeEnd;
        fragment.partStart = requestStart > wholeStart ? requestStart : wholeStart;
        fragment.partEnd = requestEnd < wholeEnd ? requestEnd : wholeEnd;
        fragment.value = mValue;
        return {fragment};
      }

    private:
      std::string mValue;
    };

    class HorizontalPatternRenderTree final : public RenderTree
    {
    public:
      explicit HorizontalPatternRenderTree(std::vector<std::string> values)
      : mValues(std::move(values))
      {
      }

      std::vector<QueryFragment> query(const QueryRequest& request) const override
      {
        if (request.start == request.end || mValues.empty())
        {
          return {};
        }

        std::vector<QueryFragment> fragments;
        const auto count = static_cast<long>(mValues.size());

        for (long i = 0; i < count; i++)
        {
          const auto wholeStart = Fraction(i, count);
          const auto wholeEnd = Fraction(i + 1, count);
          if (request.end <= wholeStart || request.start >= wholeEnd)
          {
            continue;
          }

          QueryFragment fragment;
          fragment.wholeStart = wholeStart;
          fragment.wholeEnd = wholeEnd;
          fragment.partStart = request.start > wholeStart ? request.start : wholeStart;
          fragment.partEnd = request.end < wholeEnd ? request.end : wholeEnd;
          fragment.value = mValues[static_cast<size_t>(i)];
          fragments.push_back(fragment);
        }

        return fragments;
      }

    private:
      std::vector<std::string> mValues;
    };

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

    std::optional<std::string> collapseElementValue(const rapidjson::Value& node)
    {
      if (!node.IsObject())
      {
        return std::nullopt;
      }
      if (!node.HasMember("type_") || !node["type_"].IsString())
      {
        return std::nullopt;
      }
      if (std::string(node["type_"].GetString()) != "element" || !node.HasMember("source_"))
      {
        return std::nullopt;
      }

      const auto& source = node["source_"];
      if (source.IsObject())
      {
        return collapseElementValue(source);
      }

      return sourceAsString(source);
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

    std::optional<std::vector<std::string>> horizontalPatternValues(const rapidjson::Value& v)
    {
      if (!isHorizontalPatternNode(v))
      {
        return std::nullopt;
      }

      const auto& source = v["source_"].GetArray();
      if (source.Empty())
      {
        return std::vector<std::string>{};
      }

      std::vector<std::string> values;
      values.reserve(source.Size());
      for (const auto& child : source)
      {
        const auto collapsed = collapseElementValue(child);
        if (!collapsed.has_value())
        {
          return std::nullopt;
        }
        values.push_back(collapsed.value());
      }

      return values;
    }
  }

  RenderTreePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
  {
    if (isElementNode(v))
    {
      return std::make_shared<SingleEventRenderTree>(sourceAsString(v["source_"]));
    }

    const auto patternValues = horizontalPatternValues(v);
    if (patternValues.has_value())
    {
      return std::make_shared<HorizontalPatternRenderTree>(patternValues.value());
    }

    return std::make_shared<EmptyRenderTree>();
  }
}
