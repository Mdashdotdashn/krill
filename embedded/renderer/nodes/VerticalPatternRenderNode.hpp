#pragma once

#include <utility>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class VerticalPatternRenderNode final : public RenderTree
  {
  public:
    explicit VerticalPatternRenderNode(std::vector<RenderTreePtr> children)
    : mChildren(std::move(children))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end)
      {
        return {};
      }

      std::vector<QueryFragment> fragments;
      for (const auto& child : mChildren)
      {
        if (!child)
        {
          continue;
        }

        const auto childFragments = child->query(request);
        fragments.insert(fragments.end(), childFragments.begin(), childFragments.end());
      }

      return fragments;
    }

  private:
    std::vector<RenderTreePtr> mChildren;
  };
}
