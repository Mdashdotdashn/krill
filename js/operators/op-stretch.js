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

makeStrechOperator = function(source, stretchFactor)
{
  var stretchFn = function(args)
  {
    return stretchPattern(args[0], args[1]);
  }

  return new Operator(stretchFn, [source, stretchFactor]);
}

////////////////////////////////////////////////////////////////////////////////

makeFixedStepOperator = function(source, stepDivision)
{
  var sourceWeight = source.getWeight();
  var stretchFactor = math.divide(sourceWeight, stepDivision);

  var stretchFn = function(args)
  {
    return stretchPattern(args[0], args[1]);
  }

  return new Operator(stretchFn, [source, stretchFactor]);
}
