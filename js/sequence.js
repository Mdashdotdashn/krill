const math = require("mathjs");
const _ = require("lodash");

////////////////////////////////////////////////////////////////////////////////

fracToString = function(f)
{
  return ""+f.n+"/"+f.d;
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
      return { time: fracToString(fraction), value: x };
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
  this.cycleLength_ = 0;
  this.sequence_ = [];
}

Sequence.prototype.renderArray = function(sequenceArray)
{
  var length = math.number(1);
  var rendered = renderArray(sequenceArray , length, 0);
  var grouped = rendered.reduce(function(collection, x) {
    // push and create if necessary
    (collection[x.time] = collection[x.time] ? collection[x.time]: []).push(x.value);
     return collection;}
  ,{});

  const ordered = [];
  const fractionCompareFn = (a,b) => { return math.compare(math.fraction(a), math.fraction(b))};
  Object.keys(grouped).sort(fractionCompareFn).forEach(function(key) {
    ordered.push({time: key, values : grouped[key]});
  });

  this.cycleLength_ = fracToString(math.fraction(length));
  this.sequence_ = ordered;
}

Sequence.prototype.timeAtIndex = function(index)
{
  return math.fraction(this.sequence_[index].time);
}

Sequence.prototype.nextTimeFrom = function(time)
{
  var searchTime = math.mod(time, math.fraction(this.cycleLength_));

  var minIndex = 0;
  var minTime = this.timeAtIndex(minIndex);

  while (minIndex < this.sequence_.length)
  {
    var minTime = this.timeAtIndex(minIndex);
    if (minTime > searchTime)
    {
      return minTime;
    }
    minIndex++;
  }
  return math.fraction(this.cycleLength_);
}
