assert = require("assert");
math = require("mathjs");
require('../js/input-evaluator.js');
require('../js/rendering-tree.js');

TestEvaluator = function()
{
  this.evaluator_ = new Evaluator();
  this.renderingTreeBuilder_ = new RenderingTreeBuilder();
}

TestEvaluator.prototype.evaluateRenderingTree = function(s)
{
  const model = this.evaluator_.evaluate(s);
  return this.renderingTreeBuilder_.rebuild(model);
}

TestEvaluator.prototype.evaluateSequence = function(s)
{
  const renderingTree = this.evaluateRenderingTree(s);
  return renderingTree.render();
}

evaluator = new TestEvaluator();
quote ="\"";

convertSequenceToString = function(sequence)
{
  return sequence.events_.reduce(function(a,x) { var o = new Object() ; o[x.timeString()] = x.values(); a.push(o); return a;}, []);
}

testSequenceMatches = function(sequence, expected, cycleLength)
{
  var output = convertSequenceToString(sequence);
  assert.deepEqual(output,expected);
  if (cycleLength) assert.equal(math.format(sequence.cycleLength_), cycleLength);

}
