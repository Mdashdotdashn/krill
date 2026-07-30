require("../nodes/struct-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeStructNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "struct")
      || !factoryUtils.hasArrayArguments(modelNode, 1))
  {
    return null;
  }

  return new StructRenderNode(
    buildRenderNode(modelNode.arguments_[0]),
    buildRenderNode(modelNode.source_)
  );
}

module.exports = {
  makeStructNode: makeStructNode
};
