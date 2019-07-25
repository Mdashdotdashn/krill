var math = require("mathjs");
require("./base.js");
require('../js/pattern.js');

function testNextTime(string, time, expected)
{
  var sequence = evaluator.evaluatePattern(quote + string + quote);
  // Test we've got either the expected value or 'undefined' (meaning the cycle finishing)
  var next = sequence.nextTimeFrom(math.fraction(time));
  var nextTime = next ? next.time_ : undefined;
  assert.deepEqual(nextTime, expected ? math.fraction(expected) : undefined);
}


testNextTime("1 2 3, 4 5", math.number(0.5), "2/3");
testNextTime("1 2 3", math.number(0), "1/3");
testNextTime("1 2 3", math.fraction("1/3"), "2/3");
// Next time from last element is undefined, marking the cycle's end
testNextTime("1 2 3", math.fraction("2/3"), undefined);
