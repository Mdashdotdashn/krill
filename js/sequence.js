const math = require("mathjs");
const _ = require("lodash");


////////////////////////////////////////////////////////////////////////////////

fracToString = function(f)
{
  return math.format(f);
}

////////////////////////////////////////////////////////////////////////////////

Sequence = function(array)
{
  this.cycleLength_ = math.fraction(1);
  this.sequence_ = array;
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
  return this.sequence_.length;
}

Sequence.prototype.dataAtIndex = function(index)
{
  return this.sequence_[index];
}


Sequence.prototype.timeAtIndex = function(index)
{
  return this.sequence_[index].time();
}

// This could be nicely replaced by a binary search
Sequence.prototype.nextTimeFrom = function(searchTime)
{
  var minIndex = 0;
  var minTime = this.timeAtIndex(minIndex);

  while (minIndex < this.sequence_.length)
  {
    var minTime = this.timeAtIndex(minIndex);
    if (math.compare(minTime, searchTime) > 0)
    {
      return this.sequence_[minIndex];
    }
    minIndex++;
  }
}

Sequence.prototype.valueAtTime = function(time)
{
  var searchTime = math.mod(time,this.cycleLength_);

  var minIndex = 0;

  while (minIndex < this.sequence_.length)
  {
    var current = this.timeAtIndex(minIndex);
    if (current == searchTime)
    {
      return this.sequence_[index].values;
    }
  }
}
