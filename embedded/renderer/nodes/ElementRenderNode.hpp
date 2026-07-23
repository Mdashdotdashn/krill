#pragma once

#include <utility>

#include "renderer/RenderTreeBuilder.hpp"

namespace krill
{
  class ElementRenderNode final : public RenderTree
  {
  public:
    explicit ElementRenderNode(std::string value)
    : mValue(std::move(value))
    {
    }

    explicit ElementRenderNode(RenderTreePtr sourceNode)
    : mpSourceNode(std::move(sourceNode))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end)
      {
        return {};
      }

      if (mpSourceNode)
      {
        return mpSourceNode->query(request);
      }

      const Fraction wholeStart(0);
      const Fraction wholeEnd(1);
      if (request.end <= wholeStart || request.start >= wholeEnd)
      {
        return {};
      }

      QueryFragment fragment;
      fragment.wholeStart = wholeStart;
      fragment.wholeEnd = wholeEnd;
      fragment.partStart = request.start > wholeStart ? request.start : wholeStart;
      fragment.partEnd = request.end < wholeEnd ? request.end : wholeEnd;
      fragment.value = mValue;
      return {fragment};
    }

  private:
    std::string mValue;
    RenderTreePtr mpSourceNode{};
  };
}
