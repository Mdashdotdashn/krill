const math = require("mathjs");

var stretchSequence = function(sequence, factor)
{
  f = math.fraction(factor);

  var multTime = function(t) {
    var multed = math.multiply(math.fraction(t), f);
    return fracToString(multed);
  }

  var result = new Sequence();
  result.cycleLength_ = multTime(sequence.cycleLength_);
  result.sequence_ = sequence.sequence_.map(function(x) {
    return {time : multTime(x.time), values: x.values };
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
