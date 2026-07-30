var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
