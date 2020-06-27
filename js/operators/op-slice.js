math = require("mathjs");
require("../patterns/pattern.js");

////////////////////////////////////////////////////////////////////////////////

// This operator is used to slice a sequence in 1 cycle chunks everytime
// render is called. It wraps every pattern step content so that the pattern
// content can properly be rendered at every step

PatternSlicerOperator = function(content)
{
   // the underlying content, to be sliced
    this.content_ = content;
    // the current pattern data, ready for render
    this.pattern_ = makeEmptyPattern();
    this.slicingLength_ = math.fraction(1);
    this.tick();
}

PatternSlicerOperator.prototype.tick = function()
{
  // continue to evaluate the underlying content until we have one cycle of data
  // from the current position
  while (math.compare(this.pattern_.cycleLength_, this.slicingLength_) < 0)
  {
    const pattern = this.content_.render();
    this.pattern_ = joinPattern(this.pattern_, pattern);
    this.content_.tick();
  }
}

PatternSlicerOperator.prototype.render = function()
{
//  Log("source", this.pattern_);
  const slice = slicePattern(this.pattern_, math.fraction(0), this.slicingLength_);
//  Log("slice", slice);
  if (math.compare(this.pattern_.cycleLength_, this.slicingLength_) > 0)
  {
    var remainingLength = math.subtract(this.pattern_.cycleLength_, this.slicingLength_);
    this.pattern_ = slicePattern(this.pattern_, math.fraction(1), remainingLength);
  }
  else
  {
    this.pattern_ = makeEmptyPattern();
  }
//  Log("remainder", this.pattern_);
  return slice;
}
