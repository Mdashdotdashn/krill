var utils = require("./test-utils.js");
var assert = utils.assert;
var math = utils.math;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
