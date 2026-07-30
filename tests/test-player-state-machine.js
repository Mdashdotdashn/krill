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

(function testSchedulerAndPayloadSeparation()
{
  // nextOnsetTimeFrom returns only a time — never an event payload.
  // eventsAtTime returns only values — never a next time.
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  var next = player.nextOnsetTimeFrom("0");
  assert(typeof next === "object" && next.n !== undefined, "nextOnsetTimeFrom must return a fraction, not a payload");
  assert(!Array.isArray(next), "nextOnsetTimeFrom must not return an array");

  var values = player.eventsAtTime("0");
  assert(Array.isArray(values), "eventsAtTime must return an array");
  assert(!values.n, "eventsAtTime must not return a fraction");
})();

(function testNextOnsetTimeFromMonotonicityAndOnsetExtraction()
{
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  assert.deepStrictEqual(player.eventsAtTime("0"), ["a"]);
  assertFracEq(player.nextOnsetTimeFrom("0"), "1/4");

  assert.deepStrictEqual(player.eventsAtTime("1/4"), ["b"]);
  assertFracEq(player.nextOnsetTimeFrom("1/4"), "1/2");

  assert.deepStrictEqual(player.eventsAtTime("1/2"), ["c"]);
  assertFracEq(player.nextOnsetTimeFrom("1/2"), "3/4");

  assert.deepStrictEqual(player.eventsAtTime("3/4"), ["d"]);
  assertFracEq(player.nextOnsetTimeFrom("3/4"), "1");
})();

(function testTreeReplacementAtCycleBoundary()
{
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  assert.deepStrictEqual(player.eventsAtTime("1/2"), ["c"]);

  // Queue update during the cycle.
  player.setTree(makeTree("'x y'"));

  // Old tree remains active until next cycle boundary.
  assert.deepStrictEqual(player.eventsAtTime("3/4"), ["d"]);
  assertFracEq(player.nextOnsetTimeFrom("3/4"), "1");

  // New tree is active starting at boundary.
  assert.deepStrictEqual(player.eventsAtTime("1"), ["x"]);
  assert.deepStrictEqual(player.eventsAtTime("3/2"), ["y"]);
})();

(function testCompatibilityAliasesStillWork()
{
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'bd sd'"));
  player.reset();

  assert.deepStrictEqual(player.eventsAtTime("0"), ["bd"]);
  assert.deepStrictEqual(player.eventsAtTime("1/2"), ["sd"]);
  assert.deepStrictEqual(player.eventsAtTime("1/4"), []);
  assertFracEq(player.nextOnsetTimeFrom("0"), "1/2");
})();

(function testNextOnsetTimeFromStretchedPattern()
{
  // '[a]*4' is a StretchRenderNode(1/4) whose queryArc returns one fragment per slot.
  // nextOnsetTimeFrom must use wholeEnd-driven advancement to correctly find all onsets.
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'[a]*4'"));
  player.reset();

  assertFracEq(player.nextOnsetTimeFrom(math.fraction(-1, 10000)), "0");
  assert.deepStrictEqual(player.eventsAtTime("0"), ["a"]);

  assertFracEq(player.nextOnsetTimeFrom("0"), "1/4");
  assert.deepStrictEqual(player.eventsAtTime("1/4"), ["a"]);

  assertFracEq(player.nextOnsetTimeFrom("1/4"), "1/2");
  assertFracEq(player.nextOnsetTimeFrom("1/2"), "3/4");
  assertFracEq(player.nextOnsetTimeFrom("3/4"), "1");
})();

(function testNextOnsetTimeFromDoesNotExceedLookaheadBoundary()
{
  // A rest pattern has no events. nextOnsetTimeFrom should return nextCycleBoundary, not hang.
  var player = new RenderingTreePlayer();
  player.setTree(makeTree("'~'"));
  player.reset();

  var result = player.nextOnsetTimeFrom("0");
  assert(result !== null && result !== undefined, "must return a time, not null");
  assertFracEq(result, "1");
})();
