require("../nodes/bjorklund-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeBjorklundNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "bjorklund")
      || !factoryUtils.hasArrayArguments(modelNode, 2))
  {
    return null;
  }

  return new BjorklundRenderNode(
    buildRenderNode(modelNode.source_),
    modelNode.arguments_[0],
    modelNode.arguments_[1]
  );
}

module.exports = {
  makeBjorklundNode: makeBjorklundNode
};
