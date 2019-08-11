var math = require("mathjs");
require("./base.js");
require('../js/pattern.js');

//////////////////////////////////////////////////////////////////////////////

function testSlice(string, position, length, expected)
{
  var p = evaluator.evaluatePattern(string);
  var actual = slicePattern(p, math.fraction(position), math.fraction(length));
  testSequenceMatches(actual, expected);
}

testSlice("'1 2 3 4'", "1/2", "1/2",
[
 { "0/1" : ["3"] },
 { "1/4" : ["4"] }
]
)

testSlice("'1 2 3 4'", "1/2", "1",
[
 { "0/1" : ["3"] },
 { "1/4" : ["4"] },
 { "1/2" : ["1"] },
 { "3/4" : ["2"] }
]
)

testSlice("'1 2 3 4'", "1/8", "1",
[
 { "1/8" : ["2"] },
 { "3/8" : ["3"] },
 { "5/8" : ["4"] },
 { "7/8" : ["1"] }
]
)

testSlice("slow 0.5 $ '1 2 3 4'", "0", "1",
[
  { "0/1" : ["1"] },
  { "1/8" : ["2"] },
  { "1/4" : ["3"] },
  { "3/8" : ["4"] },
  { "1/2" : ["1"] },
  { "5/8" : ["2"] },
  { "3/4" : ["3"] },
  { "7/8" : ["4"] }
])

testSlice("'a'", "0", "1",
[
  { "0/1" : ["a"] }
]
)

///////////////////////////////////////////////////////////////////////////////

function testNextTime(string, time, expected)
{
  var pattern = evaluator.evaluatePattern(quote + string + quote);
  // Test we've got either the expected value or 'undefined' (meaning the cycle finishing)
  var next = pattern.nextTimeFrom(math.fraction(time));
  var nextTime = next ? next.time_ : undefined;
  assert.deepEqual(nextTime, expected ? math.fraction(expected) : undefined);
}


testNextTime("1 2 3, 4 5", math.number(0.5), "2/3");
testNextTime("1 2 3", math.number(0), "1/3");
testNextTime("1 2 3", math.fraction("1/3"), "2/3");
// Next time from last element is undefined, marking the cycle's end
testNextTime("1 2 3", math.fraction("2/3"), undefined);
