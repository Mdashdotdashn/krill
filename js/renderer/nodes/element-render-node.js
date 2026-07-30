var math = require("mathjs");
var TimeUtils = require("../../utils/time-utils.js");
require("./query-node-utils.js");
require("./base-query-render-node.js");

ElementRenderNode = function(source)
{
  BaseQueryRenderNode.call(this);
  this.source_ = source;
}

ElementRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
ElementRenderNode.prototype.constructor = ElementRenderNode;

ElementRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  if (this.source_ && this.source_.query instanceof Function)
  {
    return this.source_.query(requestStart, requestEnd);
  }

  var cycleIndex = math.floor(requestStart);
  var localStart = TimeUtils.subtract(requestStart, cycleIndex);
  var localEnd = TimeUtils.subtract(requestEnd, cycleIndex);

  var wholeStart = TimeUtils.toFraction(0);
  var wholeEnd = TimeUtils.toFraction(1);
  var bounds = QueryNodeUtils.overlapBounds(localStart, localEnd, wholeStart, wholeEnd);
  if (!bounds)
  {
    return [];
  }

  return [this.makeFragment_(
    TimeUtils.add(wholeStart, cycleIndex),
    TimeUtils.add(wholeEnd, cycleIndex),
    TimeUtils.add(bounds.partStart, cycleIndex),
    TimeUtils.add(bounds.partEnd, cycleIndex),
    this.source_
  )];
}
