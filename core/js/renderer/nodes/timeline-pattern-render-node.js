var math = require("mathjs");
var FragmentUtils = require("../fragment-utils.js");

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

  var spans = [];
  var totalSpan = math.fraction(0);
  for (var s = 0; s < this.children_.length; s++)
  {
    var childSpan = math.fraction(1);
    var spanSource = this.children_[s];
    if (spanSource && spanSource.spanLength instanceof Function)
    {
      childSpan = QueryNodeUtils.toFraction(spanSource.spanLength());
    }
    if (math.smallerEq(childSpan, 0))
    {
      childSpan = math.fraction(1);
    }
    spans.push(childSpan);
    totalSpan = math.add(totalSpan, childSpan);
  }

  if (math.smallerEq(totalSpan, 0))
  {
    return [];
  }

  var fragments = [];
  var startCycle = math.floor(math.divide(requestStart, totalSpan));
  var endCycle = math.floor(math.divide(requestEnd, totalSpan));

  for (var cycle = startCycle; cycle <= endCycle; cycle++)
  {
    var cycleBase = math.multiply(cycle, totalSpan);
    var offset = math.fraction(0);

    for (var i = 0; i < this.children_.length; i++)
    {
      var child = this.children_[i];
      var slotStart = math.add(cycleBase, offset);
      var slotEnd = math.add(slotStart, spans[i]);
      offset = math.add(offset, spans[i]);

      var bounds = QueryNodeUtils.overlapBounds(requestStart, requestEnd, slotStart, slotEnd);
      if (!bounds || !child || !(child.query instanceof Function))
      {
        continue;
      }

      var localStart = math.subtract(bounds.partStart, slotStart);
      var localEnd = math.subtract(bounds.partEnd, slotStart);
      var childFragments = child.query(localStart, localEnd) || [];

      for (var j = 0; j < childFragments.length; j++)
      {
        var fragment = childFragments[j];
        fragments.push(FragmentUtils.remapFragmentTiming(
          fragment,
          math.add(fragment.wholeStart, slotStart),
          math.add(fragment.wholeEnd, slotStart),
          math.add(fragment.partStart, slotStart),
          math.add(fragment.partEnd, slotStart)
        ));
      }
    }
  }

  return fragments;
}
