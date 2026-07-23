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

  var wholeStart = QueryNodeUtils.toFraction(0);
  var wholeEnd = QueryNodeUtils.toFraction(1);
  var bounds = QueryNodeUtils.overlapBounds(requestStart, requestEnd, wholeStart, wholeEnd);
  if (!bounds)
  {
    return [];
  }

  return [{
    wholeStart: wholeStart,
    wholeEnd: wholeEnd,
    partStart: bounds.partStart,
    partEnd: bounds.partEnd,
    value: String(this.source_)
  }];
}
