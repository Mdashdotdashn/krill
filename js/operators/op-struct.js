const _ = require("lodash");
const math = require("mathjs");

var makeSamplingArray = function(left, right, mode)
{
  switch (mode) {
    case "left":
      return left.events_.map(x => x.time_);
    case "right":
      return right.events_.map(x => x.time_);
    default:
        const allEvents = [
            left.events_.map(x => x.time_),
            right.events_.map(x => x.time_)
          ];
      return _.chain(allEvents)
          .flatten()
          .sortBy(x => math.format(math.number(x)))
          .uniqWith((a,b) => (math.compare(a,b) == 0)).value();
  }
}

var sampleSequence = function(sequence, time)
{
  var events = sequence.events_;
//console.log("looking for time " + JSON.stringify(time));
  var index = 0;
  while (index < events.length)
  {
//console.log(index + "=>" + JSON.stringify(events[index]));
    // If we have an event past the requested time, return the values before it
    if (math.compare(events[index].time_, time) > 0)
    {
//console.log("found");
      return index > 0 ? events[index - 1].values_ : [ "~" ];
    }
    index++;
  }
  return index > 0 ? events[events.length - 1].values_ : [ "~" ];
}

// Apply an operation on the arrays containing datas for the current step
// the operation is carried on all the cobination of l & r elements
var applyOperation = function(leftArray, rightArray, operation)
{
  var result = leftArray.map(x => rightArray.reduce((c,y) => { c.push(operation(x,y)); return c}, []));
  return _.flattenDeep(result);
}

// Weaves two sequence together, i.e. sample both at given points
// and apply an operator on values. Depending on mode ("left" / "right" / "anything")
// the sampling sequence is take from leftSequence, rightSequence or the merge of both
// this is supposed to mimic |+ , +| & |+| from tidal
weaveSequences = function(leftSequence, rightSequence, mode, operation)
{
  const samplingArray =  makeSamplingArray(leftSequence, rightSequence, mode);
  const eventsArray = samplingArray.map((t) => {
    const leftValues = sampleSequence(leftSequence, t);
    const rightValues = sampleSequence(rightSequence, t);
    const applied = applyOperation(leftValues, rightValues, operation);
    return new Event(t, applied);
  });
  return makePatternFromEventArray(eventsArray);
}

var boolValue = function(b)
{
  switch (b.toLowerCase())
  {
    case "0":
    case "false":
    case "f":
    case "no":
    case "~":
      return false;
  }
  return true;
}

makeStructOperator = function(source, sequence)
{
  var applyStructFn = function(args)
  {
    var operator = function(l,r)
    {
      return boolValue(r) ? l : "~";
    }

    return weaveSequences(args[0], args[1], "right", operator);
  }

  return new Operator(applyStructFn, [source, sequence]);
}
