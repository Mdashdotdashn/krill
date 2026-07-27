require("../nodes/scale-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeScaleNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "scale")
      || !factoryUtils.hasArrayArguments(modelNode, 1))
  {
    return null;
  }

  return new ScaleRenderNode(String(modelNode.arguments_[0]), buildRenderNode(modelNode.source_));
}

module.exports = {
  makeScaleNode: makeScaleNode
};
