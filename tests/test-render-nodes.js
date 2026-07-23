var assert = require("assert");
var math = require("mathjs");

require("../js/renderer/nodes/empty-render-node.js");
require("../js/renderer/nodes/element-render-node.js");
require("../js/renderer/nodes/horizontal-pattern-render-node.js");
require("../js/renderer/nodes/vertical-pattern-render-node.js");

function fracToString(v)
{
  return math.format(math.fraction(v));
}

function F(v)
{
  return fracToString(v);
}

function fragmentToComparable(fragment)
{
  return {
    wholeStart: fracToString(fragment.wholeStart),
    wholeEnd: fracToString(fragment.wholeEnd),
    partStart: fracToString(fragment.partStart),
    partEnd: fracToString(fragment.partEnd),
    value: String(fragment.value)
  };
}

(function testEmptyRenderNode()
{
  var node = new EmptyRenderNode();
  assert.deepStrictEqual(node.query("0", "1"), []);
})();

(function testElementRenderNodeLiteral()
{
  var node = new ElementRenderNode("kick");

  var full = node.query("0", "1").map(fragmentToComparable);
  assert.deepStrictEqual(full, [{
    wholeStart: F("0"),
    wholeEnd: F("1"),
    partStart: F("0"),
    partEnd: F("1"),
    value: "kick"
  }]);

  var partial = node.query("1/4", "3/4").map(fragmentToComparable);
  assert.deepStrictEqual(partial, [{
    wholeStart: F("0"),
    wholeEnd: F("1"),
    partStart: F("1/4"),
    partEnd: F("3/4"),
    value: "kick"
  }]);

  assert.deepStrictEqual(node.query("1", "1"), []);
  assert.deepStrictEqual(node.query("2", "3"), []);
})();

(function testElementRenderNodeDelegatesToSourceNode()
{
  var calls = [];
  var sourceNode = {
    query: function(start, end)
    {
      calls.push([fracToString(start), fracToString(end)]);
      return [{
        wholeStart: math.fraction(0),
        wholeEnd: math.fraction(1),
        partStart: start,
        partEnd: end,
        value: "nested"
      }];
    }
  };

  var node = new ElementRenderNode(sourceNode);
  var result = node.query("1/8", "3/8").map(fragmentToComparable);

  assert.deepStrictEqual(calls, [["1/8", "3/8"]]);
  assert.deepStrictEqual(result, [{
    wholeStart: F("0"),
    wholeEnd: F("1"),
    partStart: F("1/8"),
    partEnd: F("3/8"),
    value: "nested"
  }]);
})();

(function testHorizontalPatternRenderNode()
{
  var node = new HorizontalPatternRenderNode([
    new ElementRenderNode("a"),
    new ElementRenderNode("b"),
    new ElementRenderNode("c")
  ]);

  var full = node.query("0", "1").map(fragmentToComparable);
  assert.deepStrictEqual(full, [
    { wholeStart: F("0"), wholeEnd: F("1/3"), partStart: F("0"), partEnd: F("1/3"), value: "a" },
    { wholeStart: F("1/3"), wholeEnd: F("2/3"), partStart: F("1/3"), partEnd: F("2/3"), value: "b" },
    { wholeStart: F("2/3"), wholeEnd: F("1"), partStart: F("2/3"), partEnd: F("1"), value: "c" }
  ]);

  var middle = node.query("1/3", "2/3").map(fragmentToComparable);
  assert.deepStrictEqual(middle, [
    { wholeStart: F("1/3"), wholeEnd: F("2/3"), partStart: F("1/3"), partEnd: F("2/3"), value: "b" }
  ]);

  var partial = node.query("1/6", "5/6").map(fragmentToComparable);
  assert.deepStrictEqual(partial, [
    { wholeStart: F("0"), wholeEnd: F("1/3"), partStart: F("1/6"), partEnd: F("1/3"), value: "a" },
    { wholeStart: F("1/3"), wholeEnd: F("2/3"), partStart: F("1/3"), partEnd: F("2/3"), value: "b" },
    { wholeStart: F("2/3"), wholeEnd: F("1"), partStart: F("2/3"), partEnd: F("5/6"), value: "c" }
  ]);
})();

(function testVerticalPatternRenderNode()
{
  var node = new VerticalPatternRenderNode([
    new ElementRenderNode("left"),
    null,
    { query: function() { return []; } },
    new ElementRenderNode("right")
  ]);

  var result = node.query("0", "1").map(fragmentToComparable);
  assert.deepStrictEqual(result, [
    { wholeStart: F("0"), wholeEnd: F("1"), partStart: F("0"), partEnd: F("1"), value: "left" },
    { wholeStart: F("0"), wholeEnd: F("1"), partStart: F("0"), partEnd: F("1"), value: "right" }
  ]);

  assert.deepStrictEqual(node.query("1/2", "1/2"), []);
})();
