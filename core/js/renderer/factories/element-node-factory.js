var math = require("mathjs");
var factoryUtils = require("./factory-utils.js");

require("../nodes/bjorklund-render-node.js");
require("../nodes/element-render-node.js");
require("../nodes/stretch-render-node.js");

function sourceUnitsForFixedStep(modelNode)
{
  if (!factoryUtils.isObjectNode(modelNode))
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
          && factoryUtils.isObjectNode(child)
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
  if (!factoryUtils.isObjectNode(operator) || !operator.type_)
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

function makeElementNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "element"))
  {
    return null;
  }

  var elementNode;
  if (modelNode.source_ && modelNode.source_ instanceof Object)
  {
    elementNode = new ElementRenderNode(buildRenderNode(modelNode.source_));
    return applyElementOperator(elementNode, modelNode);
  }

  elementNode = new ElementRenderNode(modelNode.source_);
  return applyElementOperator(elementNode, modelNode);
}

module.exports = {
  makeElementNode: makeElementNode
};
