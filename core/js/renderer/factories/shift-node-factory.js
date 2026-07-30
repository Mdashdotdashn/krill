require("../nodes/shift-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeShiftNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "shift")
      || !factoryUtils.hasArrayArguments(modelNode, 2))
  {
    return null;
  }

  var amountArg = modelNode.arguments_[0];
  var direction = modelNode.arguments_[1];
  var amountNode = factoryUtils.isObjectNode(amountArg) ? buildRenderNode(amountArg) : amountArg;
  return new ShiftRenderNode(buildRenderNode(modelNode.source_), amountNode, direction);
}

module.exports = {
  makeShiftNode: makeShiftNode
};
