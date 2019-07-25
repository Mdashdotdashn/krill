const math = require("mathjs");
const _ = require("lodash");


////////////////////////////////////////////////////////////////////////////////

fracToString = function(f)
{
  return math.format(f);
}

////////////////////////////////////////////////////////////////////////////////

var Pattern = function()
{
  this.cycleLength_ = math.fraction(1);
  this.events_ = [];
  this.targetName_ = "";
}

Pattern.prototype.clone = function()
{
  var clone = new Pattern();
  clone.events_ = this.events_.slice(0);
  clone.cycleLength_ = this.cycleLength_;
  clone.targetName_ = CloneString(this.targetName_);
  return clone;
}

Pattern.prototype.setEventArray = function(eventArray)
{
  this.events_ = eventArray;
}

Pattern.prototype.setTargetName = function(targetName)
{
  this.targetName_  = targetName;
}

Pattern.prototype.tick = function()
{
}

Pattern.prototype.render = function()
{
  return this;
}

Pattern.prototype.size = function()
{
  return this.events_.length;
}

Pattern.prototype.dataAtIndex = function(index)
{
  return this.events_[index];
}


Pattern.prototype.timeAtIndex = function(index)
{
  return this.events_[index].time();
}

// This could be nicely replaced by a binary search
Pattern.prototype.nextTimeFrom = function(searchTime)
{
  var minIndex = 0;
  var minTime = this.timeAtIndex(minIndex);

  while (minIndex < this.events_.length)
  {
    var minTime = this.timeAtIndex(minIndex);
    if (math.compare(minTime, searchTime) > 0)
    {
      return this.events_[minIndex];
    }
    minIndex++;
  }
}

Pattern.prototype.valueAtTime = function(time)
{
  var searchTime = math.mod(time,this.cycleLength_);

  var minIndex = 0;

  while (minIndex < this.events_.length)
  {
    var current = this.timeAtIndex(minIndex);
    if (current == searchTime)
    {
      return this.events_[index].values;
    }
  }
}

makePatternFromEventArray = function(array)
{
  var pattern = new Pattern();
  pattern.setEventArray(array);
  return pattern;
}
