var utils = require("./test-utils.js");
var assert = utils.assert;
var F = utils.F;
var fragmentToComparable = utils.fragmentToComparable;

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
