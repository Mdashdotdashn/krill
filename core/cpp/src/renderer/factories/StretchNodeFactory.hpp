#pragma once

#include <optional>

#include "../../third_party/rapidjson/document.h"

#include "../RenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeStretchNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<Fraction> (*valueAsFraction)(const rapidjson::Value&));
  }
}
