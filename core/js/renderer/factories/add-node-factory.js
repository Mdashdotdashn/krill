require("../nodes/add-render-node.js");
require("../nodes/element-render-node.js");
var factoryUtils = require("./factory-utils.js");
var TypeGuards = require("../../utils/type-guards.js");

function makeAddNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "add")
      || !factoryUtils.hasArrayArguments(modelNode, 1))
  {
    return null;
  }

  var arg = modelNode.arguments_[0];
  var lhs = TypeGuards.isPlainObject(arg)
    ? buildRenderNode(arg)
    : new ElementRenderNode(String(arg));

  return new AddRenderNode(lhs, buildRenderNode(modelNode.source_));
}

module.exports = {
  makeAddNode: makeAddNode
};
