// RenderTree compiles the model into query nodes once.
// Query paths delegate to the compiled node tree, not raw model inspection.
//
// Query contract:
// - query(start, end) uses a half-open window [start, end).
// - each returned fragment carries:
//   - wholeStart/wholeEnd: full interval where a value is active.
//   - partStart/partEnd: clipped intersection with the current query window.

require("./query-contract.js");

require("./nodes/empty-render-node.js");
require("./nodes/element-render-node.js");
var addNodeFactory = require("./factories/add-node-factory.js");
var bjorklundNodeFactory = require("./factories/bjorklund-node-factory.js");
var elementNodeFactory = require("./factories/element-node-factory.js");
var factoryUtils = require("./factories/factory-utils.js");
var patternNodeFactory = require("./factories/pattern-node-factory.js");
var scaleNodeFactory = require("./factories/scale-node-factory.js");
var shiftNodeFactory = require("./factories/shift-node-factory.js");
var stretchNodeFactory = require("./factories/stretch-node-factory.js");
var structNodeFactory = require("./factories/struct-node-factory.js");
var truncNodeFactory = require("./factories/trunc-node-factory.js");
var TypeGuards = require("../utils/type-guards.js");

function isElementNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "element");
}

function isHorizontalPatternNode(modelNode)
{
  return factoryUtils.hasPatternAlignment(modelNode, "h");
}

function isVerticalPatternNode(modelNode)
{
  return factoryUtils.hasPatternAlignment(modelNode, "v");
}

function isTimelinePatternNode(modelNode)
{
  return factoryUtils.hasPatternAlignment(modelNode, "t");
}

function isStretchNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "stretch")
    && factoryUtils.hasArrayArguments(modelNode, 1);
}

function isBjorklundNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "bjorklund")
    && factoryUtils.hasArrayArguments(modelNode, 2);
}

function isStructNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "struct")
    && factoryUtils.hasArrayArguments(modelNode, 1);
}

function isAddNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "add")
    && factoryUtils.hasArrayArguments(modelNode, 1);
}

function isScaleNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "scale")
    && factoryUtils.hasArrayArguments(modelNode, 1);
}

function isShiftNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "shift")
    && factoryUtils.hasArrayArguments(modelNode, 2);
}

function isTruncNode(modelNode)
{
  return factoryUtils.hasType(modelNode, "trunc")
    && factoryUtils.hasArrayArguments(modelNode, 1);
}

function makeElementNode(modelNode)
{
  return elementNodeFactory.makeElementNode(modelNode, buildRenderNode);
}

function makeHorizontalPatternNode(modelNode)
{
  return patternNodeFactory.makeHorizontalPatternNode(modelNode, buildRenderNode);
}

function makeVerticalPatternNode(modelNode)
{
  return patternNodeFactory.makeVerticalPatternNode(modelNode, buildRenderNode);
}

function makeTimelinePatternNode(modelNode)
{
  return patternNodeFactory.makeTimelinePatternNode(modelNode, buildRenderNode);
}

function makeStretchNode(modelNode)
{
  return stretchNodeFactory.makeStretchNode(modelNode, buildRenderNode);
}

function makeBjorklundNode(modelNode)
{
  return bjorklundNodeFactory.makeBjorklundNode(modelNode, buildRenderNode);
}

function makeStructNode(modelNode)
{
  return structNodeFactory.makeStructNode(modelNode, buildRenderNode);
}

function makeAddNode(modelNode)
{
  return addNodeFactory.makeAddNode(modelNode, buildRenderNode);
}

function makeScaleNode(modelNode)
{
  return scaleNodeFactory.makeScaleNode(modelNode, buildRenderNode);
}

function makeShiftNode(modelNode)
{
  return shiftNodeFactory.makeShiftNode(modelNode, buildRenderNode);
}

function makeTruncNode(modelNode)
{
  return truncNodeFactory.makeTruncNode(modelNode, buildRenderNode);
}

function buildRenderNode(modelNode)
{
  if (!TypeGuards.isPlainObject(modelNode))
  {
    return new EmptyRenderNode();
  }

  if (isElementNode(modelNode))
  {
    var elementNode = makeElementNode(modelNode);
    if (elementNode)
    {
      return elementNode;
    }
  }

  if (isHorizontalPatternNode(modelNode))
  {
    var horizontalNode = makeHorizontalPatternNode(modelNode);
    if (horizontalNode)
    {
      return horizontalNode;
    }
  }

  if (isVerticalPatternNode(modelNode))
  {
    var verticalNode = makeVerticalPatternNode(modelNode);
    if (verticalNode)
    {
      return verticalNode;
    }
  }

  if (isTimelinePatternNode(modelNode))
  {
    var timelineNode = makeTimelinePatternNode(modelNode);
    if (timelineNode)
    {
      return timelineNode;
    }
  }

  if (isStretchNode(modelNode))
  {
    var stretchNode = makeStretchNode(modelNode);
    if (stretchNode)
    {
      return stretchNode;
    }
  }

  if (isBjorklundNode(modelNode))
  {
    var bjorklundNode = makeBjorklundNode(modelNode);
    if (bjorklundNode)
    {
      return bjorklundNode;
    }
  }

  if (isStructNode(modelNode))
  {
    var structNode = makeStructNode(modelNode);
    if (structNode)
    {
      return structNode;
    }
  }

  if (isAddNode(modelNode))
  {
    var addNode = makeAddNode(modelNode);
    if (addNode)
    {
      return addNode;
    }
  }

  if (isScaleNode(modelNode))
  {
    var scaleNode = makeScaleNode(modelNode);
    if (scaleNode)
    {
      return scaleNode;
    }
  }

  if (isShiftNode(modelNode))
  {
    var shiftNode = makeShiftNode(modelNode);
    if (shiftNode)
    {
      return shiftNode;
    }
  }

  if (isTruncNode(modelNode))
  {
    var truncNode = makeTruncNode(modelNode);
    if (truncNode)
    {
      return truncNode;
    }
  }

  return new EmptyRenderNode();
}

RenderTree = function(modelNodeTree)
{
  this.rootNode_ = buildRenderNode(modelNodeTree);
}

RenderTree.prototype.query = function(start, end)
{
  return this.rootNode_.query(start, end);
}

RenderTreeBuilder = function()
{
}

RenderTreeBuilder.prototype.rebuild = function(modelNodeTree)
{
  return new RenderTree(modelNodeTree);
}

// Backward compatibility with existing JS call sites.
RenderingTreeBuilder = RenderTreeBuilder;
