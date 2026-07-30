var math = require("mathjs");
var TimeUtils = require("../../utils/time-utils.js");
require("./base-query-render-node.js");

StretchRenderNode = function(source, factor)
{
  BaseQueryRenderNode.call(this);
  this.source_ = source;
  this.factor_ = TimeUtils.toFraction(factor);
}

StretchRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
StretchRenderNode.prototype.constructor = StretchRenderNode;

StretchRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  if (!this.source_ || !(this.source_.query instanceof Function))
  {
    return [];
  }

  if (TimeUtils.equal(this.factor_, 0))
  {
    return [];
  }

  var localStart = math.divide(requestStart, this.factor_);
  var localEnd = math.divide(requestEnd, this.factor_);
  var childFragments = this.source_.query(localStart, localEnd) || [];

  return this.scaleFragments_(childFragments, this.factor_);
}

StretchRenderNode.prototype.spanLength = function()
{
  if (!this.source_ || !(this.source_.spanLength instanceof Function))
  {
    return this.factor_;
  }
  return TimeUtils.multiply(this.source_.spanLength(), this.factor_);
}
