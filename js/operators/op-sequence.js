const math = require("mathjs");
const _ = require("lodash");

require("../sequence.js");

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

// Apply a scaling factor on the element array so all elements
// fit a 1 cycles length all together
function ScaleAndOffset(eventArray)
{
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
  return makeSequenceFromEventArray(ordered);
}

//////////////////////////////////////////////////////////////////////////////

makeSequenceRenderingOperator = function(childArray, alignment)
{
  if (!Array.isArray(childArray)) throw ("Unexpected child data type");
  return new SequenceRenderingOperator(childArray, alignment);
}
