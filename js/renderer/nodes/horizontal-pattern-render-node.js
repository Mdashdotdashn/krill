var math = require("mathjs");
var TimeUtils = require("../../utils/time-utils.js");

require("./query-node-utils.js");
require("./base-query-render-node.js");

HorizontalPatternRenderNode = function(children)
{
  BaseQueryRenderNode.call(this);
  this.children_ = children || [];
  this.weights_ = [];
  for (var i = 0; i < this.children_.length; i++)
  {
    this.weights_.push(TimeUtils.toFraction(1));
  }
}

HorizontalPatternRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
HorizontalPatternRenderNode.prototype.constructor = HorizontalPatternRenderNode;

HorizontalPatternRenderNode.withWeights = function(children, weights)
{
  var node = new HorizontalPatternRenderNode(children);
  node.weights_ = [];
  for (var i = 0; i < node.children_.length; i++)
  {
    var weight = weights && weights[i] !== undefined ? weights[i] : 1;
    var asFraction = TimeUtils.toFraction(weight);
    if (math.smallerEq(asFraction, 0))
    {
      asFraction = TimeUtils.toFraction(1);
    }
    node.weights_.push(asFraction);
  }
  return node;
}

HorizontalPatternRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  if (!this.children_.length)
  {
    return [];
  }

  var fragments = [];
  var count = this.children_.length;
  var totalWeight = TimeUtils.toFraction(0);
  for (var w = 0; w < count; w++)
  {
    totalWeight = TimeUtils.add(totalWeight, this.weights_[w]);
  }

  var slotOffset = TimeUtils.toFraction(0);
  for (var index = 0; index < count; index++)
  {
    var child = this.children_[index];
    var slotWeight = this.weights_[index];
    var slotStartNormalized = math.divide(slotOffset, totalWeight);
    slotOffset = TimeUtils.add(slotOffset, slotWeight);
    var slotEndNormalized = math.divide(slotOffset, totalWeight);

    var startCycle = math.floor(requestStart);
    var endCycle = math.floor(requestEnd);

    for (var cycle = startCycle; cycle <= endCycle; cycle++)
    {
      var slotStart = TimeUtils.add(cycle, slotStartNormalized);
      var slotEnd = TimeUtils.add(cycle, slotEndNormalized);
      var slotBounds = QueryNodeUtils.overlapBounds(requestStart, requestEnd, slotStart, slotEnd);
      if (!slotBounds)
      {
        continue;
      }

      var slotSize = TimeUtils.subtract(slotEnd, slotStart);
      var localStart = TimeUtils.add(cycle, math.divide(TimeUtils.subtract(slotBounds.partStart, slotStart), slotSize));
      var localEnd = TimeUtils.add(cycle, math.divide(TimeUtils.subtract(slotBounds.partEnd, slotStart), slotSize));

      var childFragments = child.query(localStart, localEnd);
      for (var i = 0; i < childFragments.length; i++)
      {
        var fragment = childFragments[i];
        var normalizedWholeStart = TimeUtils.subtract(fragment.wholeStart, cycle);
        var normalizedWholeEnd = TimeUtils.subtract(fragment.wholeEnd, cycle);
        var normalizedPartStart = TimeUtils.subtract(fragment.partStart, cycle);
        var normalizedPartEnd = TimeUtils.subtract(fragment.partEnd, cycle);
        fragments.push(this.makeFragment_(
          TimeUtils.add(slotStart, math.multiply(normalizedWholeStart, slotSize)),
          TimeUtils.add(slotStart, math.multiply(normalizedWholeEnd, slotSize)),
          TimeUtils.add(slotStart, math.multiply(normalizedPartStart, slotSize)),
          TimeUtils.add(slotStart, math.multiply(normalizedPartEnd, slotSize)),
          fragment.value
        ));
      }
    }
  }

  return fragments;
}
