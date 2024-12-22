const math = require("mathjs");

var truncPattern = function(pattern, length)
{
  f = math.fraction(length);
  maxTime = math.multiply(f, pattern.cycleLength_);

  var clone = pattern.clone();
  clone.events_ = clone.events_.filter(function(x) {
    return x.time() < maxTime;
  });

  clone.cycleLength_ = maxTime;
  return clone;
}

////////////////////////////////////////////////////////////////////////////////

makeTruncOperator = function(source, truncSize)
{
  var truncFn = function(args)
  {
    return truncPattern(args[0], args[1]);
  }

  return new Operator(truncFn, [source, truncSize]);
}
