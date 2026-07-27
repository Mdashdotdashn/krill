var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
