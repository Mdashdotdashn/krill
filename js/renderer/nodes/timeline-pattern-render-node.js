var math = require("mathjs");

require("./query-node-utils.js");

TimelinePatternRenderNode = function(children)
{
  this.children_ = children || [];
}

TimelinePatternRenderNode.prototype.query = function(start, end)
{
  var requestStart = QueryNodeUtils.toFraction(start);
  var requestEnd = QueryNodeUtils.toFraction(end);

  if (QueryNodeUtils.hasNoWidth(requestStart, requestEnd))
  {
    return [];
  }

  if (!this.children_.length)
  {
    return [];
  }

  var cycleIndex = math.floor(requestStart);
  var count = this.children_.length;
  var childIndex = ((cycleIndex % count) + count) % count;
  var child = this.children_[childIndex];
  if (!child || !(child.query instanceof Function))
  {
    return [];
  }

  var localStart = math.subtract(requestStart, cycleIndex);
  var localEnd = math.subtract(requestEnd, cycleIndex);
  var childFragments = child.query(localStart, localEnd) || [];

  var fragments = [];
  for (var i = 0; i < childFragments.length; i++)
  {
    var fragment = childFragments[i];
    fragments.push({
      wholeStart: math.add(fragment.wholeStart, cycleIndex),
      wholeEnd: math.add(fragment.wholeEnd, cycleIndex),
      partStart: math.add(fragment.partStart, cycleIndex),
      partEnd: math.add(fragment.partEnd, cycleIndex),
      value: fragment.value
    });
  }

  return fragments;
}
