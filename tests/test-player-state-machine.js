var assert = require("assert");
var math = require("mathjs");

require("../js/input-evaluator.js");
require("../js/renderer/render-tree.js");
require("../js/playback/rendering-tree-player.js");

function F(v)
{
  return math.format(math.fraction(v));
}

function assertFracEq(actual, expected)
{
  assert(math.equal(math.fraction(actual), math.fraction(expected)), "Expected " + F(expected) + " got " + F(actual));
}

function makeTree(source)
{
  var evaluator = new Evaluator();
  var builder = new RenderingTreeBuilder();
  return builder.rebuild(evaluator.evaluate(source));
}

(function testAdvanceMonotonicityAndOnsetExtraction()
{
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  var e0 = player.eventForTime("0");
  assert.deepStrictEqual(e0.values, ["a"]);
  assertFracEq(player.advance("0"), "1/4");

  var e1 = player.eventForTime("1/4");
  assert.deepStrictEqual(e1.values, ["b"]);
  assertFracEq(player.advance("1/4"), "1/2");

  var e2 = player.eventForTime("1/2");
  assert.deepStrictEqual(e2.values, ["c"]);
  assertFracEq(player.advance("1/2"), "3/4");

  var e3 = player.eventForTime("3/4");
  assert.deepStrictEqual(e3.values, ["d"]);
  assertFracEq(player.advance("3/4"), "1");
})();

(function testTreeReplacementAtCycleBoundary()
{
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  assert.deepStrictEqual(player.eventForTime("1/2").values, ["c"]);

  // Queue update during the cycle.
  player.setTree(makeTree("'x y'"));

  // Old tree remains active until next cycle boundary.
  assert.deepStrictEqual(player.eventForTime("3/4").values, ["d"]);
  assertFracEq(player.advance("3/4"), "1");

  // New tree is active starting at boundary.
  assert.deepStrictEqual(player.eventForTime("1").values, ["x"]);
  assert.deepStrictEqual(player.eventForTime("3/2").values, ["y"]);
})();

(function testEventsForTimeAlias()
{
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'bd sd'"));
  player.reset();

  assert.deepStrictEqual(player.eventsForTime("0"), ["bd"]);
  assert.deepStrictEqual(player.eventsForTime("1/2"), ["sd"]);
  assert.deepStrictEqual(player.eventsForTime("1/4"), []);
})();
