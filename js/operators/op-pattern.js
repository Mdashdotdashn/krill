const math = require("mathjs");
const _ = require("lodash");

require("../patterns/pattern.js");

////////////////////////////////////////////////////////////////////////////////
//! holds information about characteristics (like weigth a pattern step has)

WeightedStep = function(content, weight)
{
  this.type_ = "WeightedStep";
  if (content.const_)
  {
    this.operator_ = content;
    this.const_ = true;
  }
  else
  {
    this.operator_ = new PatternSlicerOperator(content);
    this.operator_.tick(); // why prefetch is needed ?
  }
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
  var weight = (option && option.weight) ? option.weight : 1;
  return new WeightedStep(stepContent, weight);
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

var SequenceRenderingOperator = function(childArray)
{
  // look if the childs are all const and pre-render if possible
  this.type_ = "sequence";
  this.nodes_ = childArray;
}

SequenceRenderingOperator.prototype.size = function()
{
  return this.nodes_.length;
}

SequenceRenderingOperator.prototype.tick = function()
{
  this.nodes_.forEach((x) => x.tick());
}

SequenceRenderingOperator.prototype.render = function()
{
  const steps = computeEventsFromWeightArray(this.nodes_);
  return makePatternFromEventArray(steps);
}

//////////////////////////////////////////////////////////////////////////////

makePatternRenderingOperator = function(childArray)
{
  if (!Array.isArray(childArray)) throw ("Unexpected child data type");
    return new SequenceRenderingOperator(childArray);
}
