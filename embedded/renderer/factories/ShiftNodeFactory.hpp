#pragma once

#include <optional>

#include <third_party/rapidjson/document.h>

#include "renderer/RenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeShiftNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&),
      std::optional<long> (*valueAsLong)(const rapidjson::Value&));
  }
}
