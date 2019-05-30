const math = require("mathjs");

var stretchSequence = function(sequence, factor)
{
  f = math.fraction(factor);

  var multTime = function(t) {
    var multed = math.multiply(t, f);
    return multed;
  }

  var result = new Sequence();
  result.cycleLength_ = multTime(sequence.cycleLength_);
  result.sequence_ = sequence.sequence_.map(function(x) {
    return new Step(multTime(x.time()),x.values());
  })
  return result;
}

CommandEvaluator = function()
{
}

CommandEvaluator.prototype.slow = function(args, sequence)
{
  return stretchSequence(sequence, math.number(args.amount));
}
