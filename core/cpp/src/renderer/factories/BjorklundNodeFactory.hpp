#pragma once

#include <optional>

#include "../../third_party/rapidjson/document.h"

#include "../RenderNode.hpp"

namespace krill
{
  namespace factory
  {
    std::optional<RenderNodePtr> makeBjorklundNode(
      const rapidjson::Value& v,
      RenderNodePtr (*buildRenderTree)(const rapidjson::Value&),
      std::optional<long> (*valueAsLong)(const rapidjson::Value&));
  }
}
