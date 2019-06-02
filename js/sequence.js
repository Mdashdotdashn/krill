const math = require("mathjs");
const _ = require("lodash");

Step = function(time, values)
{
  this.time_ = (typeof time == 'string') ? math.fraction(time) : time;
  this.values_ = values;
}

Step.prototype.time = function()
{
  return this.time_;
}

Step.prototype.timeString = function()
{
    return math.format(this.time_);
}

Step.prototype.values = function()
{
  return this.values_;
}

////////////////////////////////////////////////////////////////////////////////

fracToString = function(f)
{
  return math.format(f);
}

////////////////////////////////////////////////////////////////////////////////

var renderVerticalData = function(data, timeScale, timeOffset)
{
//LOG  console.log("Vrender of "+JSON.stringify(data));
  // we assume all vertical data contains a horizontal containter
  var rendered = data.map((x) => renderArray(x, timeScale, timeOffset));
//LOG  console.log("Vrendered ="+JSON.stringify(rendered));
  return _.flattenDeep(rendered);
}

var renderHorizontalData = function(data, timeScale, timeOffset)
{
//LOG console.log("Hrender of "+JSON.stringify(data));
  var division = timeScale/(data.length);
  var rendered = data.map(function(x, index) {
    var position = timeOffset + division * index;
    if (typeof x === 'object')
    {
      return renderArray(x, division, position);
    }
    else
    {
      const fraction = math.fraction(position);
      return new Step(fraction,x);
    }
  }, this);
//LOG  console.log("Hrendered = "+JSON.stringify(rendered));
  return rendered;
}

var renderArray = function(sequenceArray, timeScale, timeOffset)
{
  var aligment = sequenceArray.aligment_;
  var data = sequenceArray.data_;

  switch(aligment)
  {
    case "h":
      return renderHorizontalData(data, timeScale, timeOffset);

    case "v":
      return renderVerticalData(data, timeScale, timeOffset);
  }
  throw "Unknown aligment "+ aligment;
}

////////////////////////////////////////////////////////////////////////////////

Sequence = function()
{
  this.cycleLength_ = math.fraction(0);
  this.sequence_ = [];
}

Sequence.prototype.tick = function()
{
}

Sequence.prototype.render = function()
{
  return this;
}


Sequence.prototype.renderArray = function(sequenceArray)
{
  var length = math.number(1);
  var rendered = renderArray(sequenceArray , length, 0);
  var grouped = rendered.reduce(function(collection, x) {
    // push and create if necessary
    var t = x.timeString();
    (collection[t] = collection[t] ? collection[t]: []).push(x.values_);
     return collection;}
  ,{});

  const ordered = [];
  const fractionCompareFn = (a,b) => { return math.compare(math.fraction(a), math.fraction(b))};
  Object.keys(grouped).sort(fractionCompareFn).forEach(function(key) {
    ordered.push(new Step(key,grouped[key]));
  });

  this.cycleLength_ = math.fraction(length);
  this.sequence_ = ordered;
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
