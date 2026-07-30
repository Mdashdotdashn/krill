#pragma once

#include <utility>

#include "../RenderNode.hpp"

namespace krill
{
  class VerticalPatternRenderNode final : public RenderNode
  {
  public:
    explicit VerticalPatternRenderNode(std::vector<RenderNodePtr> children)
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
    std::vector<RenderNodePtr> mChildren;
  };
}
