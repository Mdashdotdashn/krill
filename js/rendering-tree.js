var math = require('mathjs');

require("./operators/operators.js");

///////////////////////////////////////////////////////////////////////////////

// nodes can be values, arrays or nodes
function buildTreeForNode(node)
{
  // If it is an array, create an array of equivalent operator
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
  var patternArray = [new PatternEvent("0", [node])];
  return makePatternFromEventArray(patternArray);
}

buildOperator = function(type, arguments, source)
{
    switch(type)
    {
      case "scale":
  			return makeScaleOperator(source, arguments.scale);

      case "struct":
        return makeStructOperator(source, buildTreeForNode(arguments.sequence));

      case "target":
        return makeTargetOperator(source, arguments.name);

  		case "slow":
  			return makeSlowOperator(source, math.fraction(arguments.amount));

      case "bjorklund":
  			return makeBjorklundOperator(source, math.fraction(arguments.step),math.fraction(arguments.pulse));

      case "fixed-step":
      return makeFixedStepOperator(source, math.fraction(arguments.amount));

      case "sequence":
        switch(arguments.alignment)
        {
          case "h":
            return makePatternRenderingOperator(source);

          case "v":
            return makeStackRenderingOperator(source);

          case "t":
            return makeTimelineOperator(source);
        }
  	}
  	throw "Unknown operator type: " + type;
}

// Builds a rendering tree composed of operator nodes that can be
// ticked (advanced) && rendered

var buildOperatorNode = function(node)
{
  // First build the tree for the source of the current node
	var source = buildTreeForNode(node.source_);

  // Create the appropriate operator from the current type

	switch (node.type_)
	{
    case "element":
      return buildPatternStep(source, node.options_);

    default:
      return buildOperator(node.type_, node.arguments_, source);
  }
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
