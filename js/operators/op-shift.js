const math = require("mathjs");

var shiftPattern = function(pattern, amount)
{
  f = math.fraction(amount);

  var shiftTime = function(t) {
    const shifted = math.add(math.add(t, f), pattern.cycleLength_);
    const wrapped = math.mod(shifted, pattern.cycleLength_);
    return wrapped;
  }

  var clone = pattern.clone();
  clone.events_ = clone.events_.map(function(x) {
    return new PatternEvent(shiftTime(x.time()),x.values());
  })

  clone.events_.sort((a,b) => math.compare(a.time(),b.time()));
  return clone;
}

////////////////////////////////////////////////////////////////////////////////

makeShiftOperator = function(source, shiftAmount)
{
  var stretchFn = function(args)
  {
    return shiftPattern(args[0], args[1]);
  }

  return new Operator(stretchFn, [source, makeValueWrapperOperator(shiftAmount)]);
}
