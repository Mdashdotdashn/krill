#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../RenderNode.hpp"

namespace krill
{
  class StructRenderNode : public RenderNode
  {
  public:
    StructRenderNode(RenderNodePtr mask, RenderNodePtr source)
      : mpMask(std::move(mask)), mpSource(std::move(source))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (!(request.start < request.end))
      {
        return {};
      }

      const auto maskFragments = mpMask->query(request);
      std::vector<QueryFragment> out;

      for (const auto& maskFragment : maskFragments)
      {
        const auto slotStart = maskFragment.wholeStart;
        const auto slotEnd = maskFragment.wholeEnd;
        const auto partStart = request.start > slotStart ? request.start : slotStart;
        const auto partEnd = request.end < slotEnd ? request.end : slotEnd;
        if (!(partStart < partEnd))
        {
          continue;
        }

        if (isTruthyMask(maskFragment.value))
        {
          QueryRequest sourceRequest;
          sourceRequest.start = partStart;
          sourceRequest.end = partEnd;
          const auto sourceFragments = mpSource->query(sourceRequest);
          for (const auto& sourceFragment : sourceFragments)
          {
            QueryFragment mapped;
            mapped.wholeStart = slotStart;
            mapped.wholeEnd = slotEnd;
            mapped.partStart = sourceFragment.partStart;
            mapped.partEnd = sourceFragment.partEnd;
            mapped.value = sourceFragment.value;
            out.push_back(mapped);
          }
          continue;
        }

        QueryFragment rest;
        rest.wholeStart = slotStart;
        rest.wholeEnd = slotEnd;
        rest.partStart = partStart;
        rest.partEnd = partEnd;
        rest.value = "~";
        out.push_back(rest);
      }

      return out;
    }

  private:
    bool isTruthyMask(const std::string& value) const
    {
      if (value == "t" || value == "T") return true;
      if (value == "true" || value == "TRUE" || value == "True") return true;
      return value == "1";
    }

    RenderNodePtr mpMask;
    RenderNodePtr mpSource;
  };
}
