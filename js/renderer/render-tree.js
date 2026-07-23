// RenderTree compiles the model into query nodes once.
// Query paths delegate to the compiled node tree, not raw model inspection.

var math = require("mathjs");

require("./nodes/empty-render-node.js");
require("./nodes/element-render-node.js");
require("./nodes/add-render-node.js");
require("./nodes/bjorklund-render-node.js");
require("./nodes/horizontal-pattern-render-node.js");
require("./nodes/scale-render-node.js");
require("./nodes/shift-render-node.js");
require("./nodes/stretch-render-node.js");
require("./nodes/struct-render-node.js");
require("./nodes/vertical-pattern-render-node.js");
require("./nodes/timeline-pattern-render-node.js");
require("./nodes/trunc-render-node.js");

function sourceUnitsForFixedStep(modelNode)
{
  if (!modelNode || !(modelNode instanceof Object))
  {
    return math.fraction(1);
  }

  if (modelNode.type_ === "element" && modelNode.source_ && modelNode.source_ instanceof Object)
  {
    return sourceUnitsForFixedStep(modelNode.source_);
  }

  if (modelNode.type_ === "pattern"
      && modelNode.arguments_
      && modelNode.arguments_.alignment === "h"
      && Array.isArray(modelNode.source_)
      && modelNode.source_.length > 0)
  {
    var total = math.fraction(0);
    modelNode.source_.forEach(function(child)
    {
      var weight = math.fraction(1);
      if (child
          && child instanceof Object
          && child.options_
          && child.options_.weight !== undefined)
      {
        try
        {
          weight = math.fraction(child.options_.weight);
        }
        catch (err)
        {
          weight = math.fraction(1);
        }
        if (math.smallerEq(weight, 0))
        {
          weight = math.fraction(1);
        }
      }
      total = math.add(total, weight);
    });
    if (math.larger(total, 0))
    {
      return total;
    }
  }

  return math.fraction(1);
}

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

  if (operator.type_ === "fixed-step" && Array.isArray(operator.arguments_) && operator.arguments_.length > 0)
  {
    var stepSize;
    try
    {
      stepSize = math.fraction(operator.arguments_[0]);
    }
    catch (err)
    {
      return node;
    }
    if (math.smallerEq(stepSize, 0))
    {
      return node;
    }
    var units = sourceUnitsForFixedStep(modelNode.source_);
    return new StretchRenderNode(node, math.divide(units, stepSize));
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

  if (modelNode.type_ === "bjorklund"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length >= 2)
  {
    return new BjorklundRenderNode(
      buildRenderNode(modelNode.source_),
      modelNode.arguments_[0],
      modelNode.arguments_[1]
    );
  }

  if (modelNode.type_ === "struct"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length > 0)
  {
    return new StructRenderNode(
      buildRenderNode(modelNode.arguments_[0]),
      buildRenderNode(modelNode.source_)
    );
  }

  if (modelNode.type_ === "add"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length > 0)
  {
    var arg = modelNode.arguments_[0];
    var lhs = (arg && arg instanceof Object)
      ? buildRenderNode(arg)
      : new ElementRenderNode(String(arg));
    return new AddRenderNode(lhs, buildRenderNode(modelNode.source_));
  }

  if (modelNode.type_ === "scale"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length > 0)
  {
    return new ScaleRenderNode(String(modelNode.arguments_[0]), buildRenderNode(modelNode.source_));
  }

  if (modelNode.type_ === "shift"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length >= 2)
  {
    var amountArg = modelNode.arguments_[0];
    var direction = modelNode.arguments_[1];
    var amountNode = (amountArg && amountArg instanceof Object) ? buildRenderNode(amountArg) : amountArg;
    return new ShiftRenderNode(buildRenderNode(modelNode.source_), amountNode, direction);
  }

  if (modelNode.type_ === "trunc"
      && Array.isArray(modelNode.arguments_)
      && modelNode.arguments_.length > 0)
  {
    return new TruncRenderNode(buildRenderNode(modelNode.source_), modelNode.arguments_[0]);
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
