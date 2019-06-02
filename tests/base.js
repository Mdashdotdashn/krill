assert = require("assert");
math = require("mathjs");
require('../js/evaluator.js');

evaluator = new Evaluator();
quote ="\"";

convertSequenceToString = function(sequence)
{
  return sequence.sequence_.reduce(function(a,x) { var o = new Object() ; o[x.timeString()] = x.values(); a.push(o); return a;}, []);
}

testSequenceMatches = function(sequence, expected, cycleLength)
{
  var output = convertSequenceToString(sequence);
  assert.deepEqual(output,expected);
  if (cycleLength) assert.equal(math.format(sequence.cycleLength_), cycleLength);

}
