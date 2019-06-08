assert = require("assert");
math = require("mathjs");
require('../js/input-evaluator.js');
require('../js/rendering-tree.js');

TestEvaluator = function()
{
  this.evaluator_ = new Evaluator();
  this.renderingTree_ = new RenderingTree();
}

TestEvaluator.prototype.evaluate = function(s)
{
  var model = this.evaluator_.evaluate(s);
  var renderingTree = this.renderingTree_.rebuild(model);
  return renderingTree.render();
}

evaluator = new TestEvaluator();
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
