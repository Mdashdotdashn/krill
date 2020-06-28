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

  // If the node is an object, it is expected to be an operator node
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
      case "add":
  			return makeAddOperator(source, buildTreeForNode(arguments[0]));

      case "scale":
  			return makeScaleOperator(source, arguments[0]);

      case "struct":
        return makeStructOperator(source, buildTreeForNode(arguments[0]));

      case "target":
        return makeTargetOperator(source, arguments[0]);

  		case "stretch":
  			return makeStrechOperator(source, arguments[0]);

      case "shift":
  			return makeShiftOperator(source, arguments[0]);

      case "bjorklund":
  			return makeBjorklundOperator(source, arguments[0], arguments[1]);

      case "fixed-step":
      return makeFixedStepOperator(source, arguments[0]);

      case "pattern":
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
