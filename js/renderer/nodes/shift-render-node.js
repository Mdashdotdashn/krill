var math = require("mathjs");
var TimeUtils = require("../../utils/time-utils.js");
require("./base-query-render-node.js");

ShiftRenderNode = function(source, amount, direction)
{
  BaseQueryRenderNode.call(this);
  this.source_ = source;
  this.direction_ = Number(direction) || 1;
  this.amountNode_ = null;
  this.amountValue_ = TimeUtils.toFraction(0);

  if (amount && amount.query)
  {
    this.amountNode_ = amount;
  }
  else
  {
    try
    {
      this.amountValue_ = TimeUtils.toFraction(amount || 0);
    }
    catch (err)
    {
      this.amountValue_ = TimeUtils.toFraction(0);
    }
  }
}

ShiftRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
ShiftRenderNode.prototype.constructor = ShiftRenderNode;

ShiftRenderNode.prototype.resolveAmount_ = function(start)
{
  if (!this.amountNode_)
  {
    return this.amountValue_;
  }

  var epsilon = TimeUtils.epsilon();
  var fragments = this.amountNode_.query(start, TimeUtils.add(start, epsilon)) || [];
  if (!fragments.length)
  {
    return TimeUtils.toFraction(0);
  }

  try
  {
    return TimeUtils.toFraction(fragments[0].value);
  }
  catch (err)
  {
    return TimeUtils.toFraction(0);
  }
}

ShiftRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  var amount = this.resolveAmount_(requestStart);
  var delta = TimeUtils.multiply(amount, this.direction_);

  var shiftedStart = TimeUtils.subtract(requestStart, delta);
  var shiftedEnd = TimeUtils.subtract(requestEnd, delta);
  var sourceFragments = this.source_.query(shiftedStart, shiftedEnd) || [];

  return this.shiftFragments_(sourceFragments, delta);
}
