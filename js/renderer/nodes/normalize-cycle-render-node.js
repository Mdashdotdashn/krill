require("./slice-render-node.js");

// A named one-cycle slicer used to normalize expression/operator arguments.
// Inherits PatternSlicerOperator so tick() accumulates child material until
// at least one full cycle is available.
CycleNormalizeOperator = function(content)
{
  PatternSlicerOperator.call(this, content);
  this.type_ = "cycle-normalize";
  this.setSliceSize(1);
  // Player lifecycle renders before ticking at cycle boundaries.
  // Prefetch one slice so first render is valid.
  this.tick();
}

CycleNormalizeOperator.prototype = Object.create(PatternSlicerOperator.prototype);
CycleNormalizeOperator.prototype.constructor = CycleNormalizeOperator;

makeNormalizeCycleRenderNode = function(source)
{
  return new CycleNormalizeOperator(source);
}

makeCycleNormalizeOperator = makeNormalizeCycleRenderNode;
