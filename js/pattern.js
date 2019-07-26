const math = require("mathjs");
const _ = require("lodash");


////////////////////////////////////////////////////////////////////////////////

fracToString = function(f)
{
  return math.format(f);
}

////////////////////////////////////////////////////////////////////////////////

convertPatternToString = function(sequence)
{
  return sequence.events_.reduce(function(a,x) { var o = new Object() ; o[x.timeString()] = x.values(); a.push(o); return a;}, []);
}

DumpPattern = function(s)
{
  console.log(s.cycleLength_);
  console.log(JSON.stringify(convertPatternToString(s)));
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

Pattern.prototype.setEventArray = function(eventArray, length)
{
  this.cycleLength_ = length ? length : math.fraction(1);
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

////////////////////////////////////////////////////////////////////////////////
// returns the pattern resulting of joining two patterns

joinPattern = function(p1, p2)
{
  var result = p1.clone();
  var offsetFn = function(t) { return math.add(t, p2.cycleLength_);};
  p2.events_.reduce((c,x) => { c.push(x.applyTime(offsetFn)); return c;}, result.events_);
  result.cycleLength_ = math.add(p1.cycleLength_, p2.cycleLength_);
  return result;
}

slicePattern = function(pattern, position, length)
{
  var start = math.mod(position, pattern.cycleLength_);
  var end = math.add(start, length);

  var p = pattern.clone();
  while (math.compare(p.cycleLength_, end) < 0)
  {
    p = joinPattern(p,p);
  }

  var startIndex = 0;
  const eventArray = p.events_;
  while (math.compare(eventArray[startIndex].time_, start) <0)
  {
    startIndex++;
  }

  var endIndex = startIndex;

  while ((endIndex < eventArray.length) &&(math.compare(eventArray[endIndex].time_, end) <0))
  {
    endIndex++;
  }

  const offsetFn = function(t) { return math.subtract(t, start);};
  const resultArray = eventArray.slice(startIndex, endIndex);
  return makePatternFromEventArray(resultArray.map(x => x.applyTime(offsetFn)), length);
}
