const math = require("mathjs");
const _ = require("lodash");


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
  var rendered = data.map(function(x)
    {
      if (typeof x === 'object')
      {
        return renderArray(x, timeScale, timeOffset);
      }
      else
      {
        var position = timeOffset;
        if (x != '~')
        {
          const fraction = math.fraction(position);
          return new Step(fraction,x);
        }
      }
    });
//LOG  console.log("Vrendered ="+JSON.stringify(rendered));
  return _.flattenDeep(rendered);
}

var renderHorizontalData = function(data, timeScale, timeOffset)
{
//LOG console.log("Hrender of "+JSON.stringify(data));
  var division = timeScale/(data.length);
  var rendered = data.reduce(function(container, x, index) {
    var position = timeOffset + division * index;
    if (typeof x === 'object')
    {
      container.push(renderArray(x, division, position));
    }
    else
    {
      if (x != '~')
      {
        const fraction = math.fraction(position);
        container.push(new Step(fraction,x));
      }
    }
    return container;
  }, []);
//LOG  console.log("Hrendered = "+JSON.stringify(rendered));
  return rendered;
}

var renderArray = function(sequenceArray, timeScale, timeOffset)
{
  console.log(sequenceArray);
  var alignment = sequenceArray.arguments_.alignment;
  var source = sequenceArray.source_;

  switch(alignment)
  {
    case "h":
      return renderHorizontalData(source, timeScale, timeOffset);

    case "v":
      return renderVerticalData(source, timeScale, timeOffset);
  }
  throw "Unknown alignment "+ alignment;
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
