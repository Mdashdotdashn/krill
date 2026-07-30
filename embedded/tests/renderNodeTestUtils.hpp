#pragma once

#include <string>
#include <vector>

#include "renderer/RenderNode.hpp"

namespace krill::tests
{
  inline std::vector<std::string> values(const std::vector<QueryFragment>& fragments)
  {
    std::vector<std::string> out;
    out.reserve(fragments.size());
    for (const auto& fragment : fragments)
    {
      out.push_back(fragment.value);
    }
    return out;
  }
}
