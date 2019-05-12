const math = require("mathjs");
const _ = require("lodash");

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
      return { time: ""+fraction.n+"/"+fraction.d, value: x };
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

Sequence = function(sequenceArray)
{
  this.length_ = 1;
  var rendered = renderArray(sequenceArray , this.length_, 0);
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
  this.sequence_ = ordered;
}
