const math = require("mathjs");
const _ = require("lodash");


////////////////////////////////////////////////////////////////////////////////

fracToString = function(f)
{
  return math.format(f);
}

////////////////////////////////////////////////////////////////////////////////

var Sequence = function()
{
  this.cycleLength_ = math.fraction(1);
  this.events_ = [];
  this.targetName_ = "";
}

Sequence.prototype.clone = function()
{
  var clone = new Sequence();
  clone.events_ = this.events_.slice(0);
  clone.cycleLength_ = this.cycleLength_;
  clone.targetName_ = CloneString(this.targetName_);
  return clone;
}

Sequence.prototype.setEventArray = function(eventArray)
{
  this.events_ = eventArray;
}

Sequence.prototype.setTargetName = function(targetName)
{
  this.targetName_  = targetName;
}

Sequence.prototype.tick = function()
{
}

Sequence.prototype.render = function()
{
  return this;
}

Sequence.prototype.size = function()
{
  return this.events_.length;
}

Sequence.prototype.dataAtIndex = function(index)
{
  return this.events_[index];
}


Sequence.prototype.timeAtIndex = function(index)
{
  return this.events_[index].time();
}

// This could be nicely replaced by a binary search
Sequence.prototype.nextTimeFrom = function(searchTime)
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

Sequence.prototype.valueAtTime = function(time)
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

makeSequenceFromEventArray = function(array)
{
  var sequence = new Sequence();
  sequence.setEventArray(array);
  return sequence;
}
