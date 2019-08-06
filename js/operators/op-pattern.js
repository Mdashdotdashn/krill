const math = require("mathjs");
const _ = require("lodash");

require("../pattern.js");

////////////////////////////////////////////////////////////////////////////////

// Events are container for a step happening at a given time in a sequence
Event = function(time, values)
{
  this.time_ = (typeof time == 'string') ? math.fraction(time) : time;
  this.values_ = values;
}

Event.prototype.time = function()
{
  return this.time_;
}

Event.prototype.applyTime = function(f)
{
  return new Event(f(this.time_),this.values_);
}

Event.prototype.timeString = function()
{
    return math.format(this.time_);
}

Event.prototype.values = function()
{
  return this.values_;
}

////////////////////////////////////////////////////////////////////////////////
//! holds information about characteristics (like weigth a pattern step has)

var PatternStep = function(content)
{
  return { content: content, weight: 1};
}

buildPatternStep = function(content, option)
{
  var step = new PatternStep(content);

  // One option can be of applying an operator (bjorklund for example)
  if (option)
  {
    if (option.operator)
    {
      const operator = option.operator;
      step.content = buildOperator(operator.type_, operator.arguments_, content);
    }
    if (option.weight)
    {
      step.weight = option.weight;
    }
  }
  return step;
}

////////////////////////////////////////////////////////////////////////////////

// Transforms an array of weigthed values in the form { value: , weigth: }
// into a serie of event
var computeEventsFromWeightArray = function(weightArray)
{
  // Computes the total weigth
  const totalWeight = weightArray.reduce((t,x) => { return t + x.weight; }, 0);
  // Initialize the position within the sequence
  var position = math.fraction("0");
  // For every step, stretch the events inside
  const events = weightArray.map((x) => {
    // render the sequence
    const sequence = x.content.render();
    // Apply scaling and position to every contained events
    const scaleFactor = math.fraction(x.weight, totalWeight);
    const scaled = sequence.events_.map((x) => {
      // scale them
      return x.applyTime((t) => { return math.add(math.multiply(t,scaleFactor), position)});
    },[]);
    // go to the next position
    position += scaleFactor;
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

// Vertical steps are played alongside with no
// weighting
var computeVerticalSteps = function(nodes)
{
  return nodes.map((x) => x.render().events_);
}

////////////////////////////////////////////////////////////////////////////////

var SequenceRenderingOperator = function(operatorArray, alignment)
{
  this.nodes_ = operatorArray;
  this.alignment_ = alignment;
}

SequenceRenderingOperator.prototype.tick = function()
{
  this.current_ = (this.current_ + 1) % this.nodes_.length;
  // Horizontal contains PatternStep while vertical contains direct nodes
  // this is not optimal because the two aren't equivalent anymore. We temporarily(?)
  // cope with the situation by testing the step. Ideally H/V should be split
  // in pattern and stack
  this.nodes_.forEach((x) => x.content ? x.content.tick() : x.tick() );
}

SequenceRenderingOperator.prototype.render = function()
{
  // Renders concatenate sub steps as full cycles
  const steps = (this.alignment_ == "h")
      ? computeEventsFromWeightArray(this.nodes_)
      : computeVerticalSteps(this.nodes_);

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
    ordered.push(new Event(key,_.flattenDeep(grouped[key])));
  });

//  console.log('returning ' + JSON.stringify(ordered));
  return makePatternFromEventArray(ordered);
}

//////////////////////////////////////////////////////////////////////////////

makePatternRenderingOperator = function(childArray, alignment)
{
  if (!Array.isArray(childArray)) throw ("Unexpected child data type");
  return new SequenceRenderingOperator(childArray, alignment);
}
