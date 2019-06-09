assert = require("assert");
math = require("mathjs");
require('../js/input-evaluator.js');
require('../js/rendering-tree.js');

Dump = function(o)
{
  console.log(JSON.stringify(o, undefined,1));
}

TestEvaluator = function()
{
  this.evaluator_ = new Evaluator();
  this.renderingTree_ = new RenderingTree();
}

TestEvaluator.prototype.evaluateRenderingTree = function(s)
{
  const model = this.evaluator_.evaluate(s);
  return this.renderingTree_.rebuild(model);
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
  return sequence.sequence_.reduce(function(a,x) { var o = new Object() ; o[x.timeString()] = x.values(); a.push(o); return a;}, []);
}

testSequenceMatches = function(sequence, expected, cycleLength)
{
  var output = convertSequenceToString(sequence);
  assert.deepEqual(output,expected);
  if (cycleLength) assert.equal(math.format(sequence.cycleLength_), cycleLength);

}
