var assert = require("assert");
var math = require("mathjs");

require("../js/input-evaluator.js");

function F(v)
{
  return math.format(math.fraction(v));
}

(function testEvaluateRenderingTreeWorksWithoutPreloadingRenderBuilder()
{
  var evaluator = new Evaluator();
  var tree = evaluator.evaluateRenderingTree("'a b c d'");

  assert(tree && typeof tree.query === "function", "Rendering tree must expose query(start, end)");

  var fragments = tree.query(math.fraction(0), math.fraction(1));
  assert(Array.isArray(fragments), "Query should return fragments array");
  assert(fragments.length > 0, "Expected non-empty fragments for non-empty sequence");

  assert.strictEqual(F(fragments[0].wholeStart), "0/1");
})();

(function testEvaluateRenderingTreeRejectsInvalidInput()
{
  var evaluator = new Evaluator();
  assert.throws(function() {
    evaluator.evaluateRenderingTree("slow $");
  });
})();
