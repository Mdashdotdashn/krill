#pragma once

#include "SliceRenderNode.hpp"

namespace krill
{
// A named one-cycle slicer used to normalize expression/operator arguments.
// Inherits SliceRenderNode so tick() accumulates child material until at least
// one full cycle is available.
class NormalizeCycleRenderNode : public SliceRenderNode
{
public:
  explicit NormalizeCycleRenderNode(RenderNodePtr child)
    : SliceRenderNode(child)
  {
    setSliceSize(Fraction(1));
  }
};
} // namespace krill
