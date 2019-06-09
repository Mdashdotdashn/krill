var math = require('mathjs');

require("./operators/operators.js");

///////////////////////////////////////////////////////////////////////////////

// Childs can be values, arrays or nodes
function buildTreeForChild(child)
{
  if (Array.isArray(child))
  {
    return child.map((x) => buildTreeForChild(x));
  }
  if (child instanceof Object)
  {
    return buildTreeForNode(child);
  }
  // make a singe sequence out of the step
  var sequenceArray = [new Step("0", [child])];
  return new Sequence(sequenceArray);
}
// Builds a rendering tree composed of operator nodes that can be
// ticked (advanced) && rendered

function buildTreeForNode(node)
{
  // If the child os an object it is expected to be another node.
  // Otherwise, it is raw data
	var child = buildTreeForChild(node.source_);
	switch (node.type_)
	{
		case "slow":
			return makeSlowOperator(child, math.fraction(node.arguments_.amount));
    case "sequence":
      if (node.arguments_.alignment == "t")
      {
        return makeTimelineOperator(child);
      }
      return makeSequenceRenderingOperator(child, node.arguments_.alignment);
	}
	throw "Unknown model node type: " + node.type_;
}

RenderingTree = function()
{
}

RenderingTree.prototype.rebuild = function(modelNodeTree)
{
  var tree =  buildTreeForNode(modelNodeTree);
  return tree;
}