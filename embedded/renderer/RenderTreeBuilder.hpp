#pragma once

#include "RenderNode.hpp"

#include "third_party/rapidjson/document.h"

namespace krill
{
class RenderTreeBuilder
{
public:
  static RenderNodePtr fromJson(const rapidjson::Value& value);
};
}
