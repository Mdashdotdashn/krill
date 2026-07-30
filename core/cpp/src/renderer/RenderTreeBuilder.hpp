#pragma once

#include "../../third_party/rapidjson/document.h"
#include "RenderNode.hpp"

namespace krill
{
  class RenderTreeBuilder
  {
  public:
    static RenderNodePtr fromJson(const rapidjson::Value& v);
  };
}
