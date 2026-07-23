var math = require("mathjs");

require("./query-node-utils.js");

HorizontalPatternRenderNode = function(children)
{
  this.children_ = children || [];
  this.weights_ = [];
  for (var i = 0; i < this.children_.length; i++)
  {
    this.weights_.push(math.fraction(1));
  }
}

HorizontalPatternRenderNode.withWeights = function(children, weights)
{
  var node = new HorizontalPatternRenderNode(children);
  node.weights_ = [];
  for (var i = 0; i < node.children_.length; i++)
  {
    var weight = weights && weights[i] !== undefined ? weights[i] : 1;
    var asFraction = QueryNodeUtils.toFraction(weight);
    if (math.smallerEq(asFraction, 0))
    {
      asFraction = math.fraction(1);
    }
    node.weights_.push(asFraction);
  }
  return node;
}

HorizontalPatternRenderNode.prototype.query = function(start, end)
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

  var fragments = [];
  var count = this.children_.length;
  var totalWeight = math.fraction(0);
  for (var w = 0; w < count; w++)
  {
    totalWeight = math.add(totalWeight, this.weights_[w]);
  }

  var slotOffset = math.fraction(0);
  for (var index = 0; index < count; index++)
  {
    var child = this.children_[index];
    var slotWeight = this.weights_[index];
    var slotStartNormalized = math.divide(slotOffset, totalWeight);
    slotOffset = math.add(slotOffset, slotWeight);
    var slotEndNormalized = math.divide(slotOffset, totalWeight);

    var startCycle = math.floor(requestStart);
    var endCycle = math.floor(requestEnd);

    for (var cycle = startCycle; cycle <= endCycle; cycle++)
    {
      var slotStart = math.add(cycle, slotStartNormalized);
      var slotEnd = math.add(cycle, slotEndNormalized);
      var slotBounds = QueryNodeUtils.overlapBounds(requestStart, requestEnd, slotStart, slotEnd);
      if (!slotBounds)
      {
        continue;
      }

      var slotSize = math.subtract(slotEnd, slotStart);
      var localStart = math.add(cycle, math.divide(math.subtract(slotBounds.partStart, slotStart), slotSize));
      var localEnd = math.add(cycle, math.divide(math.subtract(slotBounds.partEnd, slotStart), slotSize));

      var childFragments = child.query(localStart, localEnd);
      for (var i = 0; i < childFragments.length; i++)
      {
        var fragment = childFragments[i];
        var normalizedWholeStart = math.subtract(fragment.wholeStart, cycle);
        var normalizedWholeEnd = math.subtract(fragment.wholeEnd, cycle);
        var normalizedPartStart = math.subtract(fragment.partStart, cycle);
        var normalizedPartEnd = math.subtract(fragment.partEnd, cycle);
        fragments.push({
          wholeStart: math.add(slotStart, math.multiply(normalizedWholeStart, slotSize)),
          wholeEnd: math.add(slotStart, math.multiply(normalizedWholeEnd, slotSize)),
          partStart: math.add(slotStart, math.multiply(normalizedPartStart, slotSize)),
          partEnd: math.add(slotStart, math.multiply(normalizedPartEnd, slotSize)),
          value: fragment.value
        });
      }
    }
  }

  return fragments;
}
