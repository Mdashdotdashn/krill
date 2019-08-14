const math = require("mathjs");

var stretchSequence = function(sequence, factor)
{
  f = math.fraction(factor);

  var multTime = function(t) {
    var multed = math.multiply(t, f);
    return multed;
  }

  var clone = sequence.clone();
  clone.cycleLength_ = multTime(clone.cycleLength_);
  clone.events_ = clone.events_.map(function(x) {
    return new PatternEvent(multTime(x.time()),x.values());
  })
  return clone;
}

makeSlowOperator = function(source, stretchFactor)
{
  var stretchFn = function(args)
  {
    return stretchSequence(args[0], args[1]);
  }

  return new Operator(stretchFn, [source, makeValueWrapperOperator(stretchFactor)]);
}
