var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
