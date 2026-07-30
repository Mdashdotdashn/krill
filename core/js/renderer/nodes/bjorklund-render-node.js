var math = require("mathjs");

require("./query-node-utils.js");

BjorklundRenderNode = function(source, pulses, steps)
{
  this.source_ = source;
  this.pulses_ = Math.max(0, Math.floor(Number(pulses)));
  this.steps_ = Math.max(1, Math.floor(Number(steps)));
  this.activeSlots_ = [];

  if (this.pulses_ >= this.steps_)
  {
    for (var all = 0; all < this.steps_; all++)
    {
      this.activeSlots_.push(all);
    }
    return;
  }

  for (var i = 0; i < this.steps_; i++)
  {
    if (((i * this.pulses_) % this.steps_) < this.pulses_)
    {
      this.activeSlots_.push(i);
    }
  }
}

BjorklundRenderNode.prototype.query = function(start, end)
{
  var requestStart = QueryNodeUtils.toFraction(start);
  var requestEnd = QueryNodeUtils.toFraction(end);

  if (QueryNodeUtils.hasNoWidth(requestStart, requestEnd))
  {
    return [];
  }

  if (!this.source_ || !(this.source_.query instanceof Function) || !this.activeSlots_.length)
  {
    return [];
  }

  var fragments = [];
  var stepSize = math.fraction(1, this.steps_);
  var startCycle = math.floor(requestStart);
  var endCycle = math.floor(requestEnd);

  for (var cycle = startCycle; cycle <= endCycle; cycle++)
  {
    for (var s = 0; s < this.activeSlots_.length; s++)
    {
      var slot = this.activeSlots_[s];
      var nextSlot = (s + 1 < this.activeSlots_.length)
        ? this.activeSlots_[s + 1]
        : (this.activeSlots_[0] + this.steps_);

      var slotStart = math.add(cycle, math.multiply(slot, stepSize));
      var slotEnd = math.add(cycle, math.multiply(nextSlot, stepSize));
      var bounds = QueryNodeUtils.overlapBounds(requestStart, requestEnd, slotStart, slotEnd);
      if (!bounds)
      {
        continue;
      }

      var slotSize = math.subtract(slotEnd, slotStart);
      var localStart = math.divide(math.subtract(bounds.partStart, slotStart), slotSize);
      var localEnd = math.divide(math.subtract(bounds.partEnd, slotStart), slotSize);
      var childFragments = this.source_.query(localStart, localEnd) || [];

      for (var i = 0; i < childFragments.length; i++)
      {
        var fragment = childFragments[i];
        fragments.push({
          wholeStart: math.add(slotStart, math.multiply(fragment.wholeStart, slotSize)),
          wholeEnd: math.add(slotStart, math.multiply(fragment.wholeEnd, slotSize)),
          partStart: math.add(slotStart, math.multiply(fragment.partStart, slotSize)),
          partEnd: math.add(slotStart, math.multiply(fragment.partEnd, slotSize)),
          value: fragment.value
        });
      }
    }
  }

  return fragments;
}
