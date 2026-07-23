var math = require("mathjs");

require("./query-node-utils.js");

StretchRenderNode = function(source, factor)
{
  this.source_ = source;
  this.factor_ = QueryNodeUtils.toFraction(factor);
}

StretchRenderNode.prototype.query = function(start, end)
{
  var requestStart = QueryNodeUtils.toFraction(start);
  var requestEnd = QueryNodeUtils.toFraction(end);

  if (QueryNodeUtils.hasNoWidth(requestStart, requestEnd))
  {
    return [];
  }

  if (!this.source_ || !(this.source_.query instanceof Function))
  {
    return [];
  }

  if (math.equal(this.factor_, 0))
  {
    return [];
  }

  var localStart = math.divide(requestStart, this.factor_);
  var localEnd = math.divide(requestEnd, this.factor_);
  var childFragments = this.source_.query(localStart, localEnd) || [];

  var fragments = [];
  for (var i = 0; i < childFragments.length; i++)
  {
    var fragment = childFragments[i];
    fragments.push({
      wholeStart: math.multiply(fragment.wholeStart, this.factor_),
      wholeEnd: math.multiply(fragment.wholeEnd, this.factor_),
      partStart: math.multiply(fragment.partStart, this.factor_),
      partEnd: math.multiply(fragment.partEnd, this.factor_),
      value: fragment.value
    });
  }

  return fragments;
}

StretchRenderNode.prototype.spanLength = function()
{
  if (!this.source_ || !(this.source_.spanLength instanceof Function))
  {
    return this.factor_;
  }
  return math.multiply(this.source_.spanLength(), this.factor_);
}
