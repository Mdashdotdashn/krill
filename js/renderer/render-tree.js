// RenderTree compiles the model into query nodes once.
// Query paths delegate to the compiled node tree, not raw model inspection.

require("./nodes/empty-render-node.js");
require("./nodes/element-render-node.js");
require("./nodes/bjorklund-render-node.js");
require("./nodes/horizontal-pattern-render-node.js");
require("./nodes/stretch-render-node.js");
require("./nodes/vertical-pattern-render-node.js");
require("./nodes/timeline-pattern-render-node.js");

function applyElementOperator(node, modelNode)
{
  if (!modelNode || !modelNode.options_ || !modelNode.options_.operator)
  {
    return node;
  }

  var operator = modelNode.options_.operator;
  if (!operator || !(operator instanceof Object) || !operator.type_)
  {
    return node;
  }

  if (operator.type_ === "bjorklund" && Array.isArray(operator.arguments_) && operator.arguments_.length >= 2)
  {
    return new BjorklundRenderNode(node, operator.arguments_[0], operator.arguments_[1]);
  }

  if (operator.type_ === "stretch" && Array.isArray(operator.arguments_) && operator.arguments_.length > 0)
  {
    return new StretchRenderNode(node, operator.arguments_[0]);
  }

  return node;
}

function buildRenderNode(modelNode)
{
  if (!modelNode || !(modelNode instanceof Object))
  {
    return new EmptyRenderNode();
  }

  if (modelNode.type_ === "element")
  {
    var elementNode;
    if (modelNode.source_ && modelNode.source_ instanceof Object)
    {
      elementNode = new ElementRenderNode(buildRenderNode(modelNode.source_));
      return applyElementOperator(elementNode, modelNode);
    }
    elementNode = new ElementRenderNode(modelNode.source_);
    return applyElementOperator(elementNode, modelNode);
  }

  if (modelNode.type_ === "pattern"
      && modelNode.arguments_
      && modelNode.arguments_.alignment === "h"
      && Array.isArray(modelNode.source_))
  {
    var children = modelNode.source_.map(function(child)
    {
      return buildRenderNode(child);
    });

    var weights = modelNode.source_.map(function(child)
    {
      if (!child || !(child instanceof Object) || !child.options_ || child.options_.weight === undefined)
      {
        return 1;
      }
      return child.options_.weight;
    });

    return HorizontalPatternRenderNode.withWeights(children, weights);
  }

  if (modelNode.type_ === "pattern"
      && modelNode.arguments_
      && modelNode.arguments_.alignment === "v"
      && Array.isArray(modelNode.source_))
  {
    var verticalChildren = modelNode.source_.map(function(child)
    {
      return buildRenderNode(child);
    });
    return new VerticalPatternRenderNode(verticalChildren);
  }

  if (modelNode.type_ === "pattern"
      && modelNode.arguments_
      && modelNode.arguments_.alignment === "t"
      && Array.isArray(modelNode.source_))
  {
    var timelineChildren = modelNode.source_.map(function(child)
    {
      return buildRenderNode(child);
    });
    return new TimelinePatternRenderNode(timelineChildren);
  }

  if (modelNode.type_ === "stretch"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length > 0)
  {
    return new StretchRenderNode(buildRenderNode(modelNode.source_), modelNode.arguments_[0]);
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
