#pragma once

#include "RenderNode.hpp"

namespace krill
{
  class RenderTreePlayer
  {
  public:
    void setTree(RenderNodePtr tree)
    {
      mpTree = tree;
    }

    std::vector<QueryFragment> queryArc(const Fraction& start, const Fraction& end) const
    {
      if (!mpTree)
      {
        return {};
      }
      return mpTree->query(QueryRequest{start, end});
    }

    std::vector<QueryFragment> queryPointWindow(const Fraction& time) const
    {
      return queryArc(time, time + Fraction(1, 1024));
    }

  private:
    RenderNodePtr mpTree{};
  };
}
