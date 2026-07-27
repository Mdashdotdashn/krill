require("../nodes/trunc-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeTruncNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "trunc")
      || !factoryUtils.hasArrayArguments(modelNode, 1))
  {
    return null;
  }

  return new TruncRenderNode(buildRenderNode(modelNode.source_), modelNode.arguments_[0]);
}

module.exports = {
  makeTruncNode: makeTruncNode
};
