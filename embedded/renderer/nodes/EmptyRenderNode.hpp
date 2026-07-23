#pragma once

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class EmptyRenderNode final : public RenderTree
  {
  public:
    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      (void)request;
      return {};
    }
  };
}
