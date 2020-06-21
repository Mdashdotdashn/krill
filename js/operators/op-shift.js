const math = require("mathjs");

var shiftSequence = function(sequence, amount)
{
  f = math.fraction(amount);

  var shiftTime = function(t) {
    const shifted = math.add(math.add(t, f), sequence.cycleLength_);
    const wrapped = math.mod(shifted, sequence.cycleLength_);
    return wrapped;
  }

  var clone = sequence.clone();
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
    return shiftSequence(args[0], args[1]);
  }

  return new Operator(stretchFn, [source, makeValueWrapperOperator(shiftAmount)]);
}
