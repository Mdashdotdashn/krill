#pragma once

#include "cycle/Cycle.hpp"

#include <memory>
#include <vector>

namespace krill
{
class RenderNode
{
public:
  virtual ~RenderNode() = default;

  virtual void tick() = 0;
  virtual Cycle render() = 0;
  virtual size_t stepCount()
  {
    return 1;
  }

  void setWeight(float weight) { mWeight = weight; }
  float weigth() { return mWeight; }

private:
  float mWeight{ 1 };
};

using RenderNodePtr = std::shared_ptr<RenderNode>;
using RenderNodeArray = std::vector<RenderNodePtr>;
} // namespace krill
