var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
