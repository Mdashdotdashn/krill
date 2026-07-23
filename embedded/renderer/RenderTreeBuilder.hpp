#pragma once

#include <memory>
#include <string>
#include <vector>

#include <third_party/rapidjson/document.h>

#include "cycle/Cycle.hpp"

namespace krill
{
  struct QueryRequest
  {
    Fraction start{0};
    Fraction end{0};
  };

  struct QueryFragment
  {
    Fraction wholeStart{0};
    Fraction wholeEnd{0};
    Fraction partStart{0};
    Fraction partEnd{0};
    std::string value;
  };

  class RenderTree
  {
  public:
    virtual ~RenderTree() = default;
    virtual std::vector<QueryFragment> query(const QueryRequest& request) const = 0;
    virtual Fraction spanLength() const
    {
      return Fraction(1);
    }
  };

  using RenderTreePtr = std::shared_ptr<RenderTree>;

  class RenderTreeBuilder
  {
  public:
    static RenderTreePtr fromJson(const rapidjson::Value& v);
  };
}
