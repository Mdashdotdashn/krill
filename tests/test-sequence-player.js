var assert = require("assert");

require("../js/playback/sequence-player.js");
require("../js/evaluator.js");

var player = new SequencePlayer();
var evaluator = new Evaluator();
var quote ="\"";

var testAdvance = function(time, expectedTime, expectedValues)
{
  assert.equal(fracToString(player.advance(time)), expectedTime);
  if (expectedValues)
  {
    assert.deepEqual(player.currentValues(), expectedValues)
  }
}

// empty sequence reports next time to be on th enext cycle

testAdvance("2/3", "1/1");
testAdvance("1/1", "2/1");
testAdvance("5/3", "2/1");

// setting a sequence reports the beginning of the sequence and its first value

var sequence = evaluator.evaluate(quote+"a b c"+quote);
player.setSequence(sequence);
testAdvance("11/6", "2/1", ["a"]);

// from now on, we simply cycle through the sequence
testAdvance("2/1", "7/3", ["b"]);  // 2 -> 2.(3)
testAdvance("7/3", "8/3", ["c"]);  // 2.(3) -> 2.(6)
testAdvance("8/3", "3/1", ["a"]);  // 2.(6) -> 3
testAdvance("3/1", "10/3", ["b"]); // 3 -> 3.(3)

// setting a new sequence will queue only at current sequence's end

var sequence4 = evaluator.evaluate(quote+"d  e f g"+quote);
player.setSequence(sequence4)
testAdvance("10/3", "11/3", ["c"]); // 3.(3) -> 3.(6)
testAdvance("11/3", "4/1", ["d"]); // 3.(6) -> 4 ;  we hit cycle start and queue new one
