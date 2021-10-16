const math = require("mathjs");
const _ = require("lodash");

mergeAndOrderSteps = function(steps)
{
  // Merge all data in a single array
  const merged = steps.reduce(function(collection,x) {
     return collection.concat(x);
  }, []);

  // Collect all result in a single, sorted array
  var grouped = merged.reduce(function(collection, x) {
    // push and create if necessary
    var t = x.timeString();
    (collection[t] = collection[t] ? collection[t]: []).push(x.values_);
     return collection;}
  ,{});

  const ordered = [];
  const fractionCompareFn = (a,b) => { return math.compare(math.fraction(a), math.fraction(b))};
  Object.keys(grouped).sort(fractionCompareFn).forEach(function(key) {
    ordered.push(new PatternEvent(key,_.flattenDeep(grouped[key])));
  });

  return ordered;
}

////////////////////////////////////////////////////////////////////////////////

var StackRenderingOperator = function(contentArray)
{
  this.content_ = contentArray;
  this.type_ = "stack";
}

StackRenderingOperator.prototype.size = function()
{
  return this.content_[0].size();
}

StackRenderingOperator.prototype.tick = function()
{
  this.content_.forEach((x) => x.tick());
}

StackRenderingOperator.prototype.render = function()
{
  // Renders concatenate every sequences as full cycles
  // At this point, different length aren't supported
  const steps = this.content_.map((x) => x.render().events_);
  const ordered = mergeAndOrderSteps(steps);
  return makePatternFromEventArray(ordered);
}

//////////////////////////////////////////////////////////////////////////////

makeStackRenderingOperator = function(childArray)
{
  if (!Array.isArray(childArray)) throw ("Unexpected child data type");
  return new StackRenderingOperator(childArray);
}
