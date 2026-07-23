var assert = require("assert");
var math = require("mathjs");

require("../js/renderer/nodes/empty-render-node.js");
require("../js/renderer/nodes/element-render-node.js");
require("../js/renderer/nodes/add-render-node.js");
require("../js/renderer/nodes/bjorklund-render-node.js");
require("../js/renderer/nodes/horizontal-pattern-render-node.js");
require("../js/renderer/nodes/scale-render-node.js");
require("../js/renderer/nodes/shift-render-node.js");
require("../js/renderer/nodes/stretch-render-node.js");
require("../js/renderer/nodes/struct-render-node.js");
require("../js/renderer/nodes/timeline-pattern-render-node.js");
require("../js/renderer/nodes/trunc-render-node.js");
require("../js/renderer/nodes/vertical-pattern-render-node.js");
require("../js/renderer/render-tree.js");

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
  var nextCycle = node.query("2", "3").map(fragmentToComparable);
  assert.deepStrictEqual(nextCycle, [{
    wholeStart: F("2"),
    wholeEnd: F("3"),
    partStart: F("2"),
    partEnd: F("3"),
    value: "kick"
  }]);
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

(function testHorizontalPatternRenderNodeNestedChildRegression()
{
  var nestedVertical = new VerticalPatternRenderNode([
    new ElementRenderNode("6"),
    new ElementRenderNode("C4")
  ]);

  var node = new HorizontalPatternRenderNode([
    new ElementRenderNode("1"),
    new ElementRenderNode("2"),
    nestedVertical
  ]);

  var result = node.query("2/3", "1").map(fragmentToComparable);
  assert.deepStrictEqual(result, [
    { wholeStart: F("2/3"), wholeEnd: F("1"), partStart: F("2/3"), partEnd: F("1"), value: "6" },
    { wholeStart: F("2/3"), wholeEnd: F("1"), partStart: F("2/3"), partEnd: F("1"), value: "C4" }
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

(function testTimelinePatternRenderNodeAlternatesByCycle()
{
  var node = new TimelinePatternRenderNode([
    new ElementRenderNode("3"),
    new ElementRenderNode("4")
  ]);

  var start0 = math.fraction(2, 3);
  var end0 = math.add(start0, math.fraction(1, 1024));
  var cycle0 = node.query(start0, end0).map(fragmentToComparable);
  assert.deepStrictEqual(cycle0, [
    { wholeStart: F("0"), wholeEnd: F("1"), partStart: F(start0), partEnd: F(end0), value: "3" }
  ]);

  var start1 = math.fraction(5, 3);
  var end1 = math.add(start1, math.fraction(1, 1024));
  var cycle1 = node.query(start1, end1).map(fragmentToComparable);
  assert.deepStrictEqual(cycle1, [
    { wholeStart: F("1"), wholeEnd: F("2"), partStart: F(start1), partEnd: F(end1), value: "4" }
  ]);
})();

(function testStretchRenderNode()
{
  var source = new HorizontalPatternRenderNode([
    new ElementRenderNode("1"),
    new ElementRenderNode("2"),
    new ElementRenderNode("3")
  ]);

  var node = new StretchRenderNode(source, "2");

  var start0 = math.fraction(0);
  var end0 = math.add(start0, math.fraction(1, 1024));
  var at0 = node.query(start0, end0).map(fragmentToComparable);
  assert.deepStrictEqual(at0, [
    { wholeStart: F("0"), wholeEnd: F("2/3"), partStart: F("0"), partEnd: F("1/1024"), value: "1" }
  ]);

  var start1 = math.fraction(2, 3);
  var end1 = math.add(start1, math.fraction(1, 1024));
  var atTwoThirds = node.query(start1, end1).map(fragmentToComparable);
  assert.deepStrictEqual(atTwoThirds, [
    { wholeStart: F("2/3"), wholeEnd: F("4/3"), partStart: F(start1), partEnd: F(end1), value: "2" }
  ]);
})();

(function testHorizontalPatternRenderNodeWeights()
{
  var node = HorizontalPatternRenderNode.withWeights([
    new ElementRenderNode("bd"),
    new ElementRenderNode("sd")
  ], [3, 1]);

  var at0 = node.query(math.fraction(0), math.fraction(1, 1024)).map(fragmentToComparable);
  assert.deepStrictEqual(at0, [
    { wholeStart: F("0"), wholeEnd: F("3/4"), partStart: F("0"), partEnd: F("1/1024"), value: "bd" }
  ]);

  var start = math.fraction(3, 4);
  var end = math.add(start, math.fraction(1, 1024));
  var atThreeQuarters = node.query(start, end).map(fragmentToComparable);
  assert.deepStrictEqual(atThreeQuarters, [
    { wholeStart: F("3/4"), wholeEnd: F("1"), partStart: F(start), partEnd: F(end), value: "sd" }
  ]);
})();

(function testTimelinePatternRenderNodeConcatenatesChildSpans()
{
  var left = new StretchRenderNode(new HorizontalPatternRenderNode([
    new ElementRenderNode("1"),
    new ElementRenderNode("12")
  ]), 2);

  var right = new StretchRenderNode(new HorizontalPatternRenderNode([
    new ElementRenderNode("5"),
    new ElementRenderNode("17")
  ]), 4);

  var node = new TimelinePatternRenderNode([left, right]);

  var at1 = node.query(math.fraction(1), math.add(math.fraction(1), math.fraction(1, 1024))).map(fragmentToComparable);
  assert.deepStrictEqual(at1, [
    { wholeStart: F("1"), wholeEnd: F("2"), partStart: F("1"), partEnd: F("1025/1024"), value: "12" }
  ]);

  var at2 = node.query(math.fraction(2), math.add(math.fraction(2), math.fraction(1, 1024))).map(fragmentToComparable);
  assert.deepStrictEqual(at2, [
    { wholeStart: F("2"), wholeEnd: F("4"), partStart: F("2"), partEnd: F("2049/1024"), value: "5" }
  ]);

  var at4 = node.query(math.fraction(4), math.add(math.fraction(4), math.fraction(1, 1024))).map(fragmentToComparable);
  assert.deepStrictEqual(at4, [
    { wholeStart: F("4"), wholeEnd: F("6"), partStart: F("4"), partEnd: F("4097/1024"), value: "17" }
  ]);
})();

(function testBjorklundRenderNode()
{
  var source = new HorizontalPatternRenderNode([
    new ElementRenderNode("bd"),
    new ElementRenderNode("sd")
  ]);
  var node = new BjorklundRenderNode(source, 2, 8);

  function queryAt(time)
  {
    var start = math.fraction(time);
    var end = math.add(start, math.fraction(1, 1024));
    return node.query(start, end).map(fragmentToComparable);
  }

  assert.deepStrictEqual(queryAt("0"), [
    { wholeStart: F("0"), wholeEnd: F("1/4"), partStart: F("0"), partEnd: F("1/1024"), value: "bd" }
  ]);
  assert.deepStrictEqual(queryAt("1/4"), [
    { wholeStart: F("1/4"), wholeEnd: F("1/2"), partStart: F("1/4"), partEnd: F("257/1024"), value: "sd" }
  ]);
  assert.deepStrictEqual(queryAt("1/2"), [
    { wholeStart: F("1/2"), wholeEnd: F("3/4"), partStart: F("1/2"), partEnd: F("513/1024"), value: "bd" }
  ]);
  assert.deepStrictEqual(queryAt("3/4"), [
    { wholeStart: F("3/4"), wholeEnd: F("1"), partStart: F("3/4"), partEnd: F("769/1024"), value: "sd" }
  ]);
})();

(function testFixedStepOperatorViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "element",
    source_: {
      type_: "pattern",
      arguments_: { alignment: "h" },
      source_: [
        { type_: "element", source_: "a" },
        { type_: "element", source_: "b" },
        { type_: "element", source_: "c" }
      ]
    },
    options_: {
      operator: {
        type_: "fixed-step",
        arguments_: [1]
      }
    }
  });

  function queryAt(time)
  {
    var start = math.fraction(time);
    var end = math.add(start, math.fraction(1, 1024));
    return tree.query(start, end).map(fragmentToComparable);
  }

  assert.deepStrictEqual(queryAt("0"), [
    { wholeStart: F("0"), wholeEnd: F("1"), partStart: F("0"), partEnd: F("1/1024"), value: "a" }
  ]);
  assert.deepStrictEqual(queryAt("1"), [
    { wholeStart: F("1"), wholeEnd: F("2"), partStart: F("1"), partEnd: F("1025/1024"), value: "b" }
  ]);
  assert.deepStrictEqual(queryAt("2"), [
    { wholeStart: F("2"), wholeEnd: F("3"), partStart: F("2"), partEnd: F("2049/1024"), value: "c" }
  ]);
})();

(function testTopLevelBjorklundViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "bjorklund",
    arguments_: [2, 8],
    source_: {
      type_: "element",
      source_: {
        type_: "pattern",
        arguments_: { alignment: "h" },
        source_: [
          { type_: "element", source_: "bd" },
          { type_: "element", source_: "sd" }
        ]
      }
    }
  });

  function queryAt(time)
  {
    var start = math.fraction(time);
    var end = math.add(start, math.fraction(1, 1024));
    return tree.query(start, end).map(fragmentToComparable);
  }

  assert.deepStrictEqual(queryAt("0"), [
    { wholeStart: F("0"), wholeEnd: F("1/4"), partStart: F("0"), partEnd: F("1/1024"), value: "bd" }
  ]);
  assert.deepStrictEqual(queryAt("1/4"), [
    { wholeStart: F("1/4"), wholeEnd: F("1/2"), partStart: F("1/4"), partEnd: F("257/1024"), value: "sd" }
  ]);
})();

(function testStructRenderNode()
{
  var mask = new HorizontalPatternRenderNode([
    new ElementRenderNode("t"),
    new ElementRenderNode("f"),
    new ElementRenderNode("f"),
    new ElementRenderNode("t")
  ]);
  var source = new ElementRenderNode("bd");
  var node = new StructRenderNode(mask, source);

  function queryAt(time)
  {
    var start = math.fraction(time);
    var end = math.add(start, math.fraction(1, 1024));
    return node.query(start, end).map(fragmentToComparable);
  }

  assert.deepStrictEqual(queryAt("0"), [
    { wholeStart: F("0"), wholeEnd: F("1/4"), partStart: F("0"), partEnd: F("1/1024"), value: "bd" }
  ]);
  assert.deepStrictEqual(queryAt("1/4"), [
    { wholeStart: F("1/4"), wholeEnd: F("1/2"), partStart: F("1/4"), partEnd: F("257/1024"), value: "~" }
  ]);
  assert.deepStrictEqual(queryAt("1/2"), [
    { wholeStart: F("1/2"), wholeEnd: F("3/4"), partStart: F("1/2"), partEnd: F("513/1024"), value: "~" }
  ]);
  assert.deepStrictEqual(queryAt("3/4"), [
    { wholeStart: F("3/4"), wholeEnd: F("1"), partStart: F("3/4"), partEnd: F("769/1024"), value: "bd" }
  ]);
})();

(function testTopLevelStructViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "struct",
    arguments_: [{
      type_: "pattern",
      arguments_: { alignment: "h" },
      source_: [
        { type_: "element", source_: "t" },
        { type_: "element", source_: "f" },
        { type_: "element", source_: "f" },
        { type_: "element", source_: "t" }
      ]
    }],
    source_: { type_: "element", source_: "bd" }
  });

  var out = tree.query(math.fraction("1/4"), math.fraction("1/4") + math.fraction(1, 1024)).map(fragmentToComparable);
  assert.deepStrictEqual(out, [
    { wholeStart: F("1/4"), wholeEnd: F("1/2"), partStart: F("1/4"), partEnd: F("257/1024"), value: "~" }
  ]);
})();

(function testAddRenderNode()
{
  var lhs = new ElementRenderNode("7.5");
  var rhs = new HorizontalPatternRenderNode([
    new ElementRenderNode("10"),
    new ElementRenderNode("11"),
    new ElementRenderNode("12")
  ]);
  var node = new AddRenderNode(lhs, rhs);

  var full = node.query("0", "1").map(fragmentToComparable);
  assert.deepStrictEqual(full, [
    { wholeStart: F("0"), wholeEnd: F("1/3"), partStart: F("0"), partEnd: F("1/3"), value: "17.5" },
    { wholeStart: F("1/3"), wholeEnd: F("2/3"), partStart: F("1/3"), partEnd: F("2/3"), value: "18.5" },
    { wholeStart: F("2/3"), wholeEnd: F("1"), partStart: F("2/3"), partEnd: F("1"), value: "19.5" }
  ]);
})();

(function testTopLevelAddViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "add",
    arguments_: ["1"],
    source_: { type_: "element", source_: "c1" }
  });

  var out = tree.query("0", "1").map(fragmentToComparable);
  assert.deepStrictEqual(out, [
    { wholeStart: F("0"), wholeEnd: F("1"), partStart: F("0"), partEnd: F("1"), value: "C#1" }
  ]);
})();

(function testScaleRenderNode()
{
  var source = new HorizontalPatternRenderNode([
    new ElementRenderNode("0"),
    new ElementRenderNode("1"),
    new ElementRenderNode("2"),
    new ElementRenderNode("3")
  ]);
  var node = new ScaleRenderNode("MiNor", source);

  var full = node.query("0", "1").map(fragmentToComparable);
  assert.deepStrictEqual(full, [
    { wholeStart: F("0"), wholeEnd: F("1/4"), partStart: F("0"), partEnd: F("1/4"), value: "0" },
    { wholeStart: F("1/4"), wholeEnd: F("1/2"), partStart: F("1/4"), partEnd: F("1/2"), value: "2" },
    { wholeStart: F("1/2"), wholeEnd: F("3/4"), partStart: F("1/2"), partEnd: F("3/4"), value: "3" },
    { wholeStart: F("3/4"), wholeEnd: F("1"), partStart: F("3/4"), partEnd: F("1"), value: "5" }
  ]);
})();

(function testTopLevelScaleViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "scale",
    arguments_: ["MiNor"],
    source_: {
      type_: "pattern",
      arguments_: { alignment: "h" },
      source_: [
        { type_: "element", source_: "0" },
        { type_: "element", source_: "1" },
        { type_: "element", source_: "2" },
        { type_: "element", source_: "3" }
      ]
    }
  });

  var start = math.fraction("1/2");
  var out = tree.query(start, math.add(start, math.fraction(1, 1024))).map(fragmentToComparable);
  assert.deepStrictEqual(out, [
    { wholeStart: F("1/2"), wholeEnd: F("3/4"), partStart: F("1/2"), partEnd: F("513/1024"), value: "3" }
  ]);
})();

(function testShiftRenderNode()
{
  var source = new HorizontalPatternRenderNode([
    new ElementRenderNode("bd"),
    new ElementRenderNode("~"),
    new ElementRenderNode("sd"),
    new ElementRenderNode("~")
  ]);

  var right = new ShiftRenderNode(source, "0.125", 1);
  function queryAt(node, time)
  {
    var start = math.fraction(time);
    var end = math.add(start, math.fraction(1, 1024));
    return node.query(start, end).map(fragmentToComparable);
  }

  assert.deepStrictEqual(queryAt(right, "1/8"), [
    { wholeStart: F("1/8"), wholeEnd: F("3/8"), partStart: F("1/8"), partEnd: F("129/1024"), value: "bd" }
  ]);

  var left = new ShiftRenderNode(source, "0.125", -1);
  assert.deepStrictEqual(queryAt(left, "1/8"), [
    { wholeStart: F("1/8"), wholeEnd: F("3/8"), partStart: F("1/8"), partEnd: F("129/1024"), value: "~" }
  ]);
})();

(function testTopLevelShiftWithPatternAmountViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "shift",
    arguments_: [{
      type_: "element",
      source_: {
        type_: "pattern",
        arguments_: { alignment: "t" },
        source_: [
          { type_: "element", source_: "0" },
          { type_: "element", source_: "0.125" }
        ]
      }
    }, -1],
    source_: {
      type_: "pattern",
      arguments_: { alignment: "h" },
      source_: [
        { type_: "element", source_: "bd" },
        { type_: "element", source_: "~" },
        { type_: "element", source_: "sd" },
        { type_: "element", source_: "~" }
      ]
    }
  });

  var start = math.fraction("9/8");
  var out = tree.query(start, math.add(start, math.fraction(1, 1024))).map(fragmentToComparable);
  assert.deepStrictEqual(out, [
    { wholeStart: F("9/8"), wholeEnd: F("11/8"), partStart: F("9/8"), partEnd: F("1153/1024"), value: "~" }
  ]);
})();

(function testTruncRenderNode()
{
  var source = new HorizontalPatternRenderNode([
    new ElementRenderNode("1"),
    new ElementRenderNode("2"),
    new ElementRenderNode("3"),
    new ElementRenderNode("4")
  ]);
  var node = new TruncRenderNode(source, "0.75");

  function queryAt(time)
  {
    var start = math.fraction(time);
    var end = math.add(start, math.fraction(1, 1024));
    return node.query(start, end).map(fragmentToComparable);
  }

  assert.deepStrictEqual(queryAt("0"), [
    { wholeStart: F("0"), wholeEnd: F("1/4"), partStart: F("0"), partEnd: F("1/1024"), value: "1" }
  ]);
  assert.deepStrictEqual(queryAt("3/4"), [
    { wholeStart: F("3/4"), wholeEnd: F("1"), partStart: F("3/4"), partEnd: F("769/1024"), value: "1" }
  ]);
  assert.deepStrictEqual(queryAt("1"), [
    { wholeStart: F("1"), wholeEnd: F("5/4"), partStart: F("1"), partEnd: F("1025/1024"), value: "2" }
  ]);
})();

(function testTopLevelTruncViaRenderTree()
{
  var builder = new RenderTreeBuilder();
  var tree = builder.rebuild({
    type_: "trunc",
    arguments_: [0.75],
    source_: {
      type_: "pattern",
      arguments_: { alignment: "h" },
      source_: [
        { type_: "element", source_: "1" },
        { type_: "element", source_: "2" },
        { type_: "element", source_: "3" },
        { type_: "element", source_: "4" }
      ]
    }
  });

  var start = math.fraction("5/4");
  var out = tree.query(start, math.add(start, math.fraction(1, 1024))).map(fragmentToComparable);
  assert.deepStrictEqual(out, [
    { wholeStart: F("5/4"), wholeEnd: F("3/2"), partStart: F("5/4"), partEnd: F("1281/1024"), value: "3" }
  ]);
})();
