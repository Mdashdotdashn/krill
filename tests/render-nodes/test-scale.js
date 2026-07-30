var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
