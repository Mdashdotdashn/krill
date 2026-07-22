const math = require("mathjs");

var stretchPattern = function(pattern, factor)
{
  f = math.fraction(factor);

  var multTime = function(t) {
    var multed = math.multiply(t, f);
    return multed;
  }

  var clone = pattern.clone();
  clone.cycleLength_ = multTime(clone.cycleLength_);
  clone.events_ = clone.events_.map(function(x) {
    return new PatternEvent(multTime(x.time()),x.values());
  })
  return clone;
}

////////////////////////////////////////////////////////////////////////////////

makeStretchRenderNode = function(source, stretchFactor)
{
  var stretchFn = function(args)
  {
    return stretchPattern(args[0], args[1]);
  }

  return new RenderNode(stretchFn, [source, stretchFactor], "stretch");
}

////////////////////////////////////////////////////////////////////////////////

makeFixedStepRenderNode = function(source, stepDivision)
{
  var sourceWeight = source.size();
  var stretchFactor = math.divide(sourceWeight, stepDivision);

  return makeStretchRenderNode(source, stretchFactor);
}

makeStrechOperator = makeStretchRenderNode;
makeFixedStepOperator = makeFixedStepRenderNode;
