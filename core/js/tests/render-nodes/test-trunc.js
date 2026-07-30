var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
