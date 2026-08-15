#include "RenderTreeBuilder.hpp"

#include <cctype>
#include <optional>

#include "factories/AddNodeFactory.hpp"
#include "factories/BjorklundNodeFactory.hpp"
#include "factories/ElementNodeFactory.hpp"
#include "factories/PatternNodeFactory.hpp"
#include "factories/ScaleNodeFactory.hpp"
#include "factories/ShiftNodeFactory.hpp"
#include "factories/StretchNodeFactory.hpp"
#include "factories/StructNodeFactory.hpp"
#include "factories/TruncNodeFactory.hpp"
#include "nodes/EmptyRenderNode.hpp"

#include <cmath>

namespace krill
{
  namespace
  {
    RenderNodePtr buildRenderTree(const rapidjson::Value& v);

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

    std::string formatDouble(double value)
    {
      if (std::fabs(value - std::round(value)) < 1e-9)
      {
        return std::to_string(static_cast<long>(std::llround(value)));
      }

      std::string text = std::to_string(value);
      while (!text.empty() && text.back() == '0')
      {
        text.pop_back();
      }
      if (!text.empty() && text.back() == '.')
      {
        text.pop_back();
      }
      return text;
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
        return formatDouble(source.GetDouble());
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
        const auto text = std::string(v.GetString());

        if (text.find('.') != std::string::npos)
        {
          size_t pos = 0;
          bool negative = false;
          if (!text.empty() && (text[pos] == '+' || text[pos] == '-'))
          {
            negative = text[pos] == '-';
            pos++;
          }

          const auto dot = text.find('.', pos);
          const auto intPart = text.substr(pos, dot - pos);
          const auto fracPart = text.substr(dot + 1);

          if (intPart.empty() && fracPart.empty())
          {
            return std::nullopt;
          }

          for (const char c : intPart)
          {
            if (!std::isdigit(static_cast<unsigned char>(c)))
            {
              return std::nullopt;
            }
          }
          for (const char c : fracPart)
          {
            if (!std::isdigit(static_cast<unsigned char>(c)))
            {
              return std::nullopt;
            }
          }

          long whole = 0;
          if (!intPart.empty())
          {
            whole = std::stol(intPart);
          }

          long denom = 1;
          for (size_t i = 0; i < fracPart.size(); i++)
          {
            denom *= 10;
          }

          long frac = 0;
          if (!fracPart.empty())
          {
            frac = std::stol(fracPart);
          }

          long numer = whole * denom + frac;
          if (negative)
          {
            numer = -numer;
          }

          return Fraction(numer, denom);
        }

        try
        {
          return Fraction(text);
        }
        catch (...)
        {
          return std::nullopt;
        }
      }
      return std::nullopt;
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

    bool isScaleNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "scale")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() > 0;
    }

    bool isShiftNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "shift")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() >= 2;
    }

    bool isTruncNode(const rapidjson::Value& v)
    {
      if (!v.IsObject() || !v.HasMember("type_") || !v["type_"].IsString())
      {
        return false;
      }
      if (std::string(v["type_"].GetString()) != "trunc")
      {
        return false;
      }
      if (!v.HasMember("source_") || !v.HasMember("arguments_") || !v["arguments_"].IsArray())
      {
        return false;
      }
      return v["arguments_"].GetArray().Size() > 0;
    }

    std::optional<RenderNodePtr> makeElementNode(const rapidjson::Value& v)
    {
      return factory::makeElementNode(v, buildRenderTree, sourceAsString, valueAsFraction, valueAsLong);
    }

    std::optional<RenderNodePtr> makeHorizontalPatternNode(const rapidjson::Value& v)
    {
      return factory::makeHorizontalPatternNode(v, buildRenderTree, valueAsFraction);
    }

    std::optional<RenderNodePtr> makeVerticalPatternNode(const rapidjson::Value& v)
    {
      return factory::makeVerticalPatternNode(v, buildRenderTree);
    }

    std::optional<RenderNodePtr> makeTimelinePatternNode(const rapidjson::Value& v)
    {
      return factory::makeTimelinePatternNode(v, buildRenderTree);
    }

    std::optional<RenderNodePtr> makeStretchNode(const rapidjson::Value& v)
    {
      return factory::makeStretchNode(v, buildRenderTree, valueAsFraction);
    }

    std::optional<RenderNodePtr> makeStructNode(const rapidjson::Value& v)
    {
      return factory::makeStructNode(v, buildRenderTree);
    }

    std::optional<RenderNodePtr> makeAddNode(const rapidjson::Value& v)
    {
      return factory::makeAddNode(v, buildRenderTree, sourceAsString);
    }

    std::optional<RenderNodePtr> makeScaleNode(const rapidjson::Value& v)
    {
      return factory::makeScaleNode(v, buildRenderTree, sourceAsString);
    }

    std::optional<RenderNodePtr> makeShiftNode(const rapidjson::Value& v)
    {
      return factory::makeShiftNode(v, buildRenderTree, valueAsFraction, valueAsLong);
    }

    std::optional<RenderNodePtr> makeTruncNode(const rapidjson::Value& v)
    {
      return factory::makeTruncNode(v, buildRenderTree, valueAsFraction);
    }

    RenderNodePtr buildRenderTree(const rapidjson::Value& v)
    {
      if (isElementNode(v))
      {
        const auto node = makeElementNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isHorizontalPatternNode(v))
      {
        const auto node = makeHorizontalPatternNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isVerticalPatternNode(v))
      {
        const auto node = makeVerticalPatternNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isTimelinePatternNode(v))
      {
        const auto node = makeTimelinePatternNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isStretchNode(v))
      {
        const auto node = makeStretchNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isBjorklundNode(v))
      {
        const auto node = factory::makeBjorklundNode(v, buildRenderTree, valueAsLong);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isStructNode(v))
      {
        const auto node = makeStructNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isAddNode(v))
      {
        const auto node = makeAddNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isScaleNode(v))
      {
        const auto node = makeScaleNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isShiftNode(v))
      {
        const auto node = makeShiftNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      if (isTruncNode(v))
      {
        const auto node = makeTruncNode(v);
        if (node.has_value())
        {
          return node.value();
        }
      }

      return std::make_shared<EmptyRenderNode>();
    }
  }

  RenderNodePtr RenderTreeBuilder::fromJson(const rapidjson::Value& v)
  {
    return buildRenderTree(v);
  }
}
