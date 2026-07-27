var assert = require("assert");
var math = require("mathjs");

require("../../js/renderer/nodes/empty-render-node.js");
require("../../js/renderer/nodes/element-render-node.js");
require("../../js/renderer/nodes/add-render-node.js");
require("../../js/renderer/nodes/bjorklund-render-node.js");
require("../../js/renderer/nodes/horizontal-pattern-render-node.js");
require("../../js/renderer/nodes/scale-render-node.js");
require("../../js/renderer/nodes/shift-render-node.js");
require("../../js/renderer/nodes/stretch-render-node.js");
require("../../js/renderer/nodes/struct-render-node.js");
require("../../js/renderer/nodes/timeline-pattern-render-node.js");
require("../../js/renderer/nodes/trunc-render-node.js");
require("../../js/renderer/nodes/vertical-pattern-render-node.js");
require("../../js/renderer/render-tree.js");

function fracToString(v)
{
  return math.format(math.fraction(v));
}

function F(v)
{
  return fracToString(v);
}

function fragmentToComparable(fragment)
{
  return {
    wholeStart: fracToString(fragment.wholeStart),
    wholeEnd: fracToString(fragment.wholeEnd),
    partStart: fracToString(fragment.partStart),
    partEnd: fracToString(fragment.partEnd),
    value: String(fragment.value)
  };
}

module.exports = {
  assert: assert,
  math: math,
  fracToString: fracToString,
  F: F,
  fragmentToComparable: fragmentToComparable
};
