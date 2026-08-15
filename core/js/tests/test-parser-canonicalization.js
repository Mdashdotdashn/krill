const assert = require("assert");
require("../input-evaluator.js");

function testTopLevelTimingCanonicalization()
{
  const evaluator = new Evaluator();

  const slow = evaluator.evaluate("slow 2 $ 'a b'");
  assert.equal(slow.type_, "stretch");
  assert.deepEqual(slow.arguments_, [2]);

  const fast = evaluator.evaluate("fast 4 $ 'a b'");
  assert.equal(fast.type_, "stretch");
  assert.deepEqual(fast.arguments_, ["1/4"]);
}

function testSliceModifierCanonicalization()
{
  const evaluator = new Evaluator();

  const slowSlice = evaluator.evaluate('"[a]/2"');
  assert.equal(slowSlice.options_.operator.type_, "stretch");
  assert.deepEqual(slowSlice.options_.operator.arguments_, [2]);

  const fastSlice = evaluator.evaluate('"[a]*4"');
  assert.equal(fastSlice.options_.operator.type_, "stretch");
  assert.deepEqual(fastSlice.options_.operator.arguments_, ["1/4"]);

  const fixedStepSlice = evaluator.evaluate('"[a]%3"');
  assert.equal(fixedStepSlice.options_.operator.type_, "fixed-step");
  assert.deepEqual(fixedStepSlice.options_.operator.arguments_, [3]);

  const velocitySlice = evaluator.evaluate('"bd:0.8"');
  assert.equal(velocitySlice.controls_.velocity, 0.8);

  const velocitySubCycle = evaluator.evaluate('"[bd sd]:100"');
  assert.equal(velocitySubCycle.controls_.velocity, 100);
}

testTopLevelTimingCanonicalization();
testSliceModifierCanonicalization();
