var assert = require("assert");

require("../js/playback/sequence-player.js");
require("./base.js");

var player = new SequencePlayer();

var testAdvance = function(time, expectedTime, expectedValues)
{
  assert.equal(fracToString(player.advance(time)), expectedTime);
  const event = player.eventForTime(expectedTime);
  if (expectedValues !== undefined)
  {
    assert.deepEqual(event.values, expectedValues)
  }
}

var updateInput = function(s)
{
  const renderingTree = evaluator.evaluateRenderingTree(s);
  player.setRenderingTree(renderingTree);
}

function TestPlaybackTiming()
{
  // empty sequence reports next time to be the next cycle's beginning

  testAdvance("2/3", "1/1");
  testAdvance("1/1", "2/1");
  testAdvance("5/3", "2/1");

  // setting a sequence reports the beginning of the sequence and its first value

  updateInput(quote+"a b c"+quote);
  testAdvance("11/6", "2/1", ["a"]);

  // from now on, we simply cycle through the sequence
  testAdvance("2/1", "7/3", ["b"]);  // 2 -> 2.(3)
  testAdvance("7/3", "8/3", ["c"]);  // 2.(3) -> 2.(6)
  testAdvance("8/3", "3/1", ["a"]);  // 2.(6) -> 3
  testAdvance("3/1", "10/3", ["b"]); // 3 -> 3.(3)

  // setting a new sequence will queue only at current sequence's end

  updateInput("slow 2 $ "+quote+"d  e f g"+quote);

  testAdvance("10/3", "11/3", ["c"]); // 3.(3) -> 3.(6); this is still playing the old cycle
  testAdvance("11/3", "4/1", ["d"]); // 3.(6) -> 4 ;  we hit cycle start and queue new one
  testAdvance("4/1", "9/2", ["e"]); // 4 -> 4.5; the slowed down cycle length_ should be taken into account
}

TestPlaybackTiming();
