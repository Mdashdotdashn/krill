var math = require('mathjs');

require("./operators/operators.js");

///////////////////////////////////////////////////////////////////////////////

// nodes can be values, arrays or nodes
function buildTreeForNode(node)
{
  // If it is an array, create an area of equivalent operator
  if (Array.isArray(node))
  {
    return node.map((x) => buildTreeForNode(x));
  }

  // If the node is an object, it is expected to be an operator note
  if (node instanceof Object)
  {
    return buildOperatorNode(node);
  }

  // make a single sequence out of the event
  var sequenceArray = [new Event("0", [node])];
  return makeSequenceFromEventArray(sequenceArray);
}

// Builds a rendering tree composed of operator nodes that can be
// ticked (advanced) && rendered

function buildOperatorNode(node)
{
  // First build the tree for the source of the current node
	var source = buildTreeForNode(node.source_);

  // Create the appropriate operator from the current type
	switch (node.type_)
	{
    case "scale":
			return makeScaleOperator(source, node.arguments_.scale);

    case "struct":
      return makeStructOperator(source, buildTreeForNode(node.arguments_.sequence));

    case "target":
      return makeTargetOperator(source, node.arguments_.name);

		case "slow":
			return makeSlowOperator(source, math.fraction(node.arguments_.amount));

    case "bjorklund":
			return makeBjorklundOperator(source, math.fraction(node.arguments_.step),math.fraction(node.arguments_.pulse));

    case "sequence":
      if (node.arguments_.alignment == "t")
      {
        return makeTimelineOperator(source);
      }
      return makeSequenceRenderingOperator(source, node.arguments_.alignment);
	}
	throw "Unknown model node type: " + node.type_;
}

////////////////////////////////////////////////////////////////////////////////
// Render tree builder. Constructs the operator tree from the model data

RenderingTreeBuilder = function()
{
}

// Rebuilds a whole tree from the model tree
RenderingTreeBuilder.prototype.rebuild = function(modelNodeTree)
{
  var tree =  buildTreeForNode(modelNodeTree);
  return tree;
}
