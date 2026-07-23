var math = require("mathjs");

ShiftRenderNode = function(source, amount, direction)
{
  this.source_ = source;
  this.direction_ = Number(direction) || 1;
  this.amountNode_ = null;
  this.amountValue_ = math.fraction(0);

  if (amount && amount.query)
  {
    this.amountNode_ = amount;
  }
  else
  {
    try
    {
      this.amountValue_ = math.fraction(amount || 0);
    }
    catch (err)
    {
      this.amountValue_ = math.fraction(0);
    }
  }
}

ShiftRenderNode.prototype.resolveAmount_ = function(start)
{
  if (!this.amountNode_)
  {
    return this.amountValue_;
  }

  var epsilon = math.fraction(1, 1024);
  var fragments = this.amountNode_.query(start, math.add(start, epsilon)) || [];
  if (!fragments.length)
  {
    return math.fraction(0);
  }

  try
  {
    return math.fraction(fragments[0].value);
  }
  catch (err)
  {
    return math.fraction(0);
  }
}

ShiftRenderNode.prototype.query = function(start, end)
{
  var requestStart = math.fraction(start);
  var requestEnd = math.fraction(end);
  if (math.equal(requestStart, requestEnd))
  {
    return [];
  }

  var amount = this.resolveAmount_(requestStart);
  var delta = math.multiply(amount, this.direction_);

  var shiftedStart = math.subtract(requestStart, delta);
  var shiftedEnd = math.subtract(requestEnd, delta);
  var sourceFragments = this.source_.query(shiftedStart, shiftedEnd) || [];

  return sourceFragments.map(function(fragment)
  {
    return {
      wholeStart: math.add(fragment.wholeStart, delta),
      wholeEnd: math.add(fragment.wholeEnd, delta),
      partStart: math.add(fragment.partStart, delta),
      partEnd: math.add(fragment.partEnd, delta),
      value: fragment.value
    };
  });
}
