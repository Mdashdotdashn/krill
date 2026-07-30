var math = require("mathjs");

TruncRenderNode = function(source, length)
{
  this.source_ = source;
  this.length_ = math.fraction(length || 0);
}

TruncRenderNode.prototype.spanLength = function()
{
  if (!(this.source_ && this.source_.spanLength instanceof Function))
  {
    return this.length_;
  }
  return math.multiply(this.source_.spanLength(), this.length_);
}

TruncRenderNode.prototype.query = function(start, end)
{
  var requestStart = math.fraction(start);
  var requestEnd = math.fraction(end);
  if (!math.smaller(requestStart, requestEnd) || math.smallerEq(this.length_, 0))
  {
    return [];
  }

  var span = this.spanLength();
  var startCycle = math.floor(math.divide(requestStart, span));
  var endCycle = math.floor(math.divide(requestEnd, span));
  var out = [];

  for (var cycle = startCycle; cycle <= endCycle; cycle++)
  {
    var cycleBase = math.multiply(cycle, span);
    var cycleEnd = math.add(cycleBase, span);
    var overlapStart = math.max(requestStart, cycleBase);
    var overlapEnd = math.min(requestEnd, cycleEnd);
    if (!math.smaller(overlapStart, overlapEnd))
    {
      continue;
    }

    var localStart = math.subtract(overlapStart, cycleBase);
    var localEnd = math.subtract(overlapEnd, cycleBase);
    var sourceFragments = this.source_.query(localStart, localEnd) || [];
    for (var i = 0; i < sourceFragments.length; i++)
    {
      var fragment = sourceFragments[i];
      out.push({
        wholeStart: math.add(fragment.wholeStart, cycleBase),
        wholeEnd: math.add(fragment.wholeEnd, cycleBase),
        partStart: math.add(fragment.partStart, cycleBase),
        partEnd: math.add(fragment.partEnd, cycleBase),
        value: fragment.value
      });
    }
  }

  return out;
}
