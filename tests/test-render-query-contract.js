var assert = require("assert");
var math = require("mathjs");

require("../js/renderer/nodes/empty-render-node.js");
require("../js/renderer/nodes/element-render-node.js");

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

(function testEmptyRenderNodeReturnsNoFragments()
{
  var node = new EmptyRenderNode();
  assert.deepStrictEqual(node.query("0", "1"), []);
})();

(function testElementRenderNodeFillsFragmentMetadata()
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
  var nextCycle = node.query("2", "3").map(fragmentToComparable);
  assert.deepStrictEqual(nextCycle, [{
    wholeStart: F("2"),
    wholeEnd: F("3"),
    partStart: F("2"),
    partEnd: F("3"),
    value: "kick"
  }]);
})();

(function testElementRenderNodeForwardsQueryWindow()
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