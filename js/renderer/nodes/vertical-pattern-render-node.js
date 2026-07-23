require("./query-node-utils.js");

VerticalPatternRenderNode = function(children)
{
  this.children_ = children || [];
}

VerticalPatternRenderNode.prototype.query = function(start, end)
{
  var requestStart = QueryNodeUtils.toFraction(start);
  var requestEnd = QueryNodeUtils.toFraction(end);

  if (QueryNodeUtils.hasNoWidth(requestStart, requestEnd))
  {
    return [];
  }

  var fragments = [];
  for (var i = 0; i < this.children_.length; i++)
  {
    var child = this.children_[i];
    if (!child || !(child.query instanceof Function))
    {
      continue;
    }

    var childFragments = child.query(requestStart, requestEnd) || [];
    for (var j = 0; j < childFragments.length; j++)
    {
      fragments.push(childFragments[j]);
    }
  }

  return fragments;
}
