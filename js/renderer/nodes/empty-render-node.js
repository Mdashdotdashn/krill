require("./query-node-utils.js");

EmptyRenderNode = function()
{
}

EmptyRenderNode.prototype.query = function(start, end)
{
  var requestStart = QueryNodeUtils.toFraction(start);
  var requestEnd = QueryNodeUtils.toFraction(end);
  if (QueryNodeUtils.hasNoWidth(requestStart, requestEnd))
  {
    return [];
  }
  return [];
}
