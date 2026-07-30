require("../nodes/stretch-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeStretchNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasType(modelNode, "stretch")
      || !factoryUtils.hasArrayArguments(modelNode, 1))
  {
    return null;
  }

  return new StretchRenderNode(buildRenderNode(modelNode.source_), modelNode.arguments_[0]);
}

module.exports = {
  makeStretchNode: makeStretchNode
};
