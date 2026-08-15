#pragma once

#include <memory>
#include <map>
#include <string>
#include <vector>

#include "../../third_party/FractionClass/Fraction.hpp"

namespace krill
{
  // Query a half-open time window [start, end). Implementations may return
  // zero or more fragments that intersect this request.
  struct QueryRequest
  {
    Fraction start{0};
    Fraction end{0};
  };

  // One intersecting piece of a rendered value.
  //
  // - wholeStart/wholeEnd: the full interval where this value is active in
  //   the node timeline.
  // - partStart/partEnd: the clipped sub-interval returned for the current
  //   QueryRequest.
  //
  // Example:
  // request  [1/4, 3/4)
  // value    [0/1, 1/1)
  // fragment whole = [0/1, 1/1), part = [1/4, 3/4)
  struct QueryFragment
  {
    Fraction wholeStart{0};
    Fraction wholeEnd{0};
    Fraction partStart{0};
    Fraction partEnd{0};
    std::string value;
    std::map<std::string, std::string> controls;
  };

  inline QueryFragment remapFragmentTiming(
    const QueryFragment& source,
    const Fraction& wholeStart,
    const Fraction& wholeEnd,
    const Fraction& partStart,
    const Fraction& partEnd)
  {
    QueryFragment mapped;
    mapped.wholeStart = wholeStart;
    mapped.wholeEnd = wholeEnd;
    mapped.partStart = partStart;
    mapped.partEnd = partEnd;
    mapped.value = source.value;
    mapped.controls = source.controls;
    return mapped;
  }

  class RenderNode
  {
  public:
    virtual ~RenderNode() = default;

    // Returns all value fragments intersecting request [start, end).
    // Fragments should preserve both the original activation window
    // (wholeStart/wholeEnd) and the clipped intersection (partStart/partEnd).
    virtual std::vector<QueryFragment> query(const QueryRequest& request) const = 0;

    // Logical span of one node cycle in timeline units.
    virtual Fraction spanLength() const
    {
      return Fraction(1);
    }
  };

  using RenderNodePtr = std::shared_ptr<RenderNode>;
}