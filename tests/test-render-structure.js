require('./base.js');

function buildTree(source)
{
  var localEvaluator = new Evaluator();
  var builder = new RenderingTreeBuilder();
  return builder.rebuild(localEvaluator.evaluate(source));
}

function testAddExpressionArgumentIsNormalized()
{
  var tree = buildTree("add (fast 2 $ '0 1') $ '2 3'");
  assert.equal(tree.arguments_[1].type_, "cycle-normalize");
}

function testStructExpressionArgumentIsNormalized()
{
  var tree = buildTree("struct (fast 2 $ 't f') $ 'bd ~ sd ~'");
  assert.equal(tree.arguments_[1].type_, "cycle-normalize");
}

function testShiftExpressionArgumentIsNormalized()
{
  var tree = buildTree("rotR (fast 2 $ '0 0.125') $ 'bd ~ sd ~'");
  assert.equal(tree.arguments_[1].type_, "cycle-normalize");
}

function testShiftScalarArgumentStaysDirectPattern()
{
  var tree = buildTree("rotR 0.25 $ 'bd ~ sd ~'");
  assert.notEqual(tree.arguments_[1].type_, "cycle-normalize");
  assert.equal(tree.arguments_[1].value_, 0.25);
}

testAddExpressionArgumentIsNormalized();
testStructExpressionArgumentIsNormalized();
testShiftExpressionArgumentIsNormalized();
testShiftScalarArgumentStaysDirectPattern();
