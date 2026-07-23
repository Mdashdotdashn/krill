var math = require("mathjs");

require("./query-node-utils.js");

ElementRenderNode = function(source)
{
  this.source_ = source;
}

ElementRenderNode.prototype.query = function(start, end)
{
  var requestStart = QueryNodeUtils.toFraction(start);
  var requestEnd = QueryNodeUtils.toFraction(end);

  if (QueryNodeUtils.hasNoWidth(requestStart, requestEnd))
  {
    return [];
  }

  if (this.source_ && this.source_.query instanceof Function)
  {
    return this.source_.query(requestStart, requestEnd);
  }

  var cycleIndex = math.floor(requestStart);
  var localStart = math.subtract(requestStart, cycleIndex);
  var localEnd = math.subtract(requestEnd, cycleIndex);

  var wholeStart = QueryNodeUtils.toFraction(0);
  var wholeEnd = QueryNodeUtils.toFraction(1);
  var bounds = QueryNodeUtils.overlapBounds(localStart, localEnd, wholeStart, wholeEnd);
  if (!bounds)
  {
    return [];
  }

  return [{
    wholeStart: math.add(wholeStart, cycleIndex),
    wholeEnd: math.add(wholeEnd, cycleIndex),
    partStart: math.add(bounds.partStart, cycleIndex),
    partEnd: math.add(bounds.partEnd, cycleIndex),
    value: String(this.source_)
  }];
}
