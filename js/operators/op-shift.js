const math = require("mathjs");
const _ = require("lodash");

var shiftPattern = function(pattern, amount)
{
  var f = math.fraction(amount);

  var shiftTime = function(t) {
    const shifted = math.add(math.add(t, f), pattern.cycleLength_);
    const wrapped = math.mod(shifted, pattern.cycleLength_);
    return wrapped;
  }

  var clone = pattern.clone();
  clone.events_ = clone.events_.map(function(x) {
    return new PatternEvent(shiftTime(x.time()),x.values());
  })

  clone.events_.sort((a,b) => math.compare(a.time(),b.time()));
  return clone;
}

var makeSamplingArray = function(left, right)
{
  const allEvents = [
    left.events_.map(x => x.time_),
    right.events_.map(x => x.time_)
  ];

  return _.chain(allEvents)
    .flatten()
    .sortBy(x => math.format(math.number(x)))
    .uniqWith((a, b) => (math.compare(a, b) == 0))
    .value();
}

var samplePattern = function(pattern, time)
{
  var events = pattern.events_;
  var index = 0;

  while (index < events.length)
  {
    if (math.compare(events[index].time_, time) > 0)
    {
      return index > 0 ? events[index - 1].values_ : ["~"];
    }
    index++;
  }

  return index > 0 ? events[events.length - 1].values_ : ["~"];
}

var mergeAndSortByTime = function(events)
{
  events.sort((a, b) => math.compare(a.time(), b.time()));

  var merged = [];
  events.forEach((event) => {
    const last = merged[merged.length - 1];
    if (last && math.compare(last.time(), event.time()) == 0)
    {
      last.values_ = last.values_.concat(event.values());
    }
    else
    {
      merged.push(new PatternEvent(event.time(), event.values().slice()));
    }
  });

  return merged;
}

var shiftPatternByPattern = function(pattern, amountPattern, direction)
{
  const sampleTimes = makeSamplingArray(pattern, amountPattern);
  const directionFactor = math.fraction(direction);
  const generatedEvents = [];

  sampleTimes.forEach((time) => {
    const sourceValues = samplePattern(pattern, time);
    const amountValues = samplePattern(amountPattern, time);

    sourceValues.forEach((sourceValue) => {
      amountValues.forEach((amountValue) => {
        var amount = math.fraction(0);
        try
        {
          amount = math.multiply(math.fraction(amountValue), directionFactor);
        }
        catch (_)
        {
          amount = math.fraction(0);
        }

        const shifted = math.mod(
          math.add(math.add(time, amount), pattern.cycleLength_),
          pattern.cycleLength_);
        generatedEvents.push(new PatternEvent(shifted, [sourceValue]));
      });
    });
  });

  return makePatternFromEventArray(mergeAndSortByTime(generatedEvents), pattern.cycleLength_);
}

////////////////////////////////////////////////////////////////////////////////

makeShiftOperator = function(source, shiftAmount, direction)
{
  var stretchFn = function(args)
  {
    const sourcePattern = args[0];
    const amountArg = args[1];
    const directionArg = args.length > 2 ? args[2] : 1;

    if (amountArg && amountArg.type_ == "pattern")
    {
      return shiftPatternByPattern(sourcePattern, amountArg, directionArg);
    }

    return shiftPattern(sourcePattern, math.multiply(math.fraction(amountArg), directionArg));
  }

  return new Operator(stretchFn, [source, shiftAmount, direction ? direction : 1]);
}
