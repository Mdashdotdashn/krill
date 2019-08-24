const math = require("mathjs");
const _ = require("lodash");

require("../pattern.js");

////////////////////////////////////////////////////////////////////////////////

// This operator is used to slice a sequence in 1 cycle chunks everytime
// render is called. It wraps every pattern step content so that the pattern
// content can properly be rendered at every step

var PatternSliceOperator = function(content)
{
    this.content_ = content; // the underlying content
    this.pattern_ = makeEmptyPattern();
    this.tick();
}

PatternSliceOperator.prototype.tick = function()
{
  // continue to evaluate the underlying content until we have one cycle of data
  // from the current position
  while (math.compare(this.pattern_.cycleLength_, math.fraction(1)) < 0)
  {
    const pattern = this.content_.render();
    this.pattern_ = joinPattern(this.pattern_, pattern);
    this.content_.tick();
  }
}

PatternSliceOperator.prototype.render = function()
{
//  Log("source", this.pattern_);
  const slice = slicePattern(this.pattern_, math.fraction(0), math.fraction(1));
//  Log("slice", slice);
  if (math.compare(this.pattern_.cycleLength_, math.fraction(1)) > 0)
  {
    var remainingLength = math.subtract(this.pattern_.cycleLength_, math.fraction(1));
    this.pattern_ = slicePattern(this.pattern_, math.fraction(1), remainingLength);
  }
  else
  {
    this.pattern_ = makeEmptyPattern();
  }
//  Log("remainder", this.pattern_);
  return slice;
}

////////////////////////////////////////////////////////////////////////////////
//! holds information about characteristics (like weigth a pattern step has)

WeightedStep = function(content, weight)
{
  this.operator_ = new PatternSliceOperator(content);
  this.weight_ = weight ? weight : 1;
}

WeightedStep.prototype.tick = function()
{
  this.operator_.tick();
}

WeightedStep.prototype.render = function()
{
  return this.operator_.render();
}

buildPatternStep = function(content, option)
{
  // apply any step based operator
  var stepContent = (option && option.operator) ? buildOperator(option.operator.type_, option.operator.arguments_, content) : content;

  var step = new WeightedStep(stepContent);

  // apply scaling options
  if (option)
  {
    if (option.weight)
    {
      step.weight_ = option.weight;
    }
  }
  return step;
}

////////////////////////////////////////////////////////////////////////////////

// Transforms an array of WeightedStep
// into a serie of event
var computeEventsFromWeightArray = function(weightArray)
{
  // Computes the total weigth
  const totalWeight = weightArray.reduce((t,x) => { return t + x.weight_; }, 0);
  // Initialize the position within the sequence
  var position = math.fraction("0");
  // For every step, stretch the events inside
  const events = weightArray.map((x) => {
    CHECK_TYPE(x, WeightedStep);
    // render the sequence
    const sequence = x.render();
//    Log("sequence render",sequence);
    // Apply scaling and position to every contained events
    const scaleFactor = math.fraction(x.weight_, totalWeight);
    const scaled = sequence.events_.map((x) => {
      // scale them
      return x.applyTime((t) => { return math.add(math.multiply(t,scaleFactor), position)});
    },[]);
    // go to the next position
    position = math.add(position, scaleFactor);
    return scaled;
  });

  // Merge all steps in a single array
  return _.flatten(events);
}

makePatternFromWeightArray = function(weigthedArray)
{
  const eventArray = computeEventsFromWeightArray(weigthedArray);
  return makePatternFromEventArray(eventArray);
}

////////////////////////////////////////////////////////////////////////////////

var SequenceRenderingOperator = function(operatorArray)
{
  this.nodes_ = operatorArray;
}

SequenceRenderingOperator.prototype.getWeight = function()
{
  const totalWeight = this.nodes_.reduce((t,x) => { return t + x.weight_; }, 0);
  return totalWeight;  
}

SequenceRenderingOperator.prototype.tick = function()
{
  this.nodes_.forEach((x) => x.tick());
}

SequenceRenderingOperator.prototype.render = function()
{
  const steps = computeEventsFromWeightArray(this.nodes_);

  // // Merge all data in a single array
  // const merged = steps.reduce(function(collection,x) {
  //    return collection.concat(x);
  // }, []);
  //
  // // Collect all result in a single, sorted array
  // var grouped = merged.reduce(function(collection, x) {
  //   // push and create if necessary
  //   var t = x.timeString();
  //   (collection[t] = collection[t] ? collection[t]: []).push(x.values_);
  //    return collection;}
  // ,{});
  //
  // const ordered = [];
  // const fractionCompareFn = (a,b) => { return math.compare(math.fraction(a), math.fraction(b))};
  // Object.keys(grouped).sort(fractionCompareFn).forEach(function(key) {
  //   ordered.push(new PatternEvent(key,_.flattenDeep(grouped[key])));
  // });
  //

  return makePatternFromEventArray(steps);
}

//////////////////////////////////////////////////////////////////////////////

makePatternRenderingOperator = function(childArray)
{
  if (!Array.isArray(childArray)) throw ("Unexpected child data type");
  return new SequenceRenderingOperator(childArray);
}
