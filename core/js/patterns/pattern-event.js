const math = require("mathjs")

// PatternEvents are container for a step happening at a given time in a sequence

PatternEvent = function(time, values)
{
  this.time_ = (typeof time == 'string') ? math.fraction(time) : time;
  this.values_ = values;
}

PatternEvent.prototype.time = function()
{
  return this.time_;
}

PatternEvent.prototype.applyTime = function(f)
{
  return new PatternEvent(f(this.time_),this.values_);
}

PatternEvent.prototype.timeString = function()
{
    return math.format(this.time_);
}

PatternEvent.prototype.values = function()
{
  return this.values_;
}
