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
    const sequence = x.sequence.render();
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
// Horizontal steps are played one after another and are squeezed together
// to fit in a cycle
var computeHorizontalSteps = function(nodes)
{
  // render the nodes
  const renders = nodes.map((x) => x.render());

  // since there can be complex operations here, we use the length of
  // the sequence as a weighting factor. That way, we can use the length
  // to implement "bd@3 sd"

  const totalWeigth = renders.reduce((total, x) => total + x.cycleLength_, 0);

  var offset = math.fraction(0);
  // every step is an array whose elements need to be scaled
  return scaled = renders.map((s) => {
    // Walk the render's event element
    const scaleFactor = math.fraction(s.cycleLength_ + "/" + totalWeigth);
    result = s.events_.map((x) => {
      // scale them
      return x.applyTime((t) => { return math.add(math.multiply(t,scaleFactor), offset)});
    },[]);
    offset = math.add(offset ,scaleFactor);
    return result;
  })
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
  this.nodes_.forEach((x) => x.tick());
}

SequenceRenderingOperator.prototype.render = function()
{
  // Renders concatenate sub steps as full cycles
  const steps = (this.alignment_ == "h")
      ? computeHorizontalSteps(this.nodes_)
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

makePatternyRenderingOperator = function(childArray, alignment)
{
  if (!Array.isArray(childArray)) throw ("Unexpected child data type");
  return new SequenceRenderingOperator(childArray, alignment);
}
