var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
