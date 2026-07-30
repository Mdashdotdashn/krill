#pragma once

#include "../RenderNode.hpp"

namespace krill
{
  class EmptyRenderNode final : public RenderNode
  {
  public:
    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      (void)request;
      return {};
    }
  };
}
