var utils = require("./test-utils.js");
var assert = utils.assert;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
