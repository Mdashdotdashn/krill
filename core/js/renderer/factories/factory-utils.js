var TypeGuards = require("../../utils/type-guards.js");

function isObjectNode(modelNode)
{
  return TypeGuards.isPlainObject(modelNode);
}

function hasArrayArguments(modelNode, minimumCount)
{
  return isObjectNode(modelNode)
    && Array.isArray(modelNode.arguments_)
    && modelNode.arguments_.length >= minimumCount;
}

function hasType(modelNode, type)
{
  return isObjectNode(modelNode) && modelNode.type_ === type;
}

function hasPatternAlignment(modelNode, alignment)
{
  return hasType(modelNode, "pattern")
    && isObjectNode(modelNode.arguments_)
    && modelNode.arguments_.alignment === alignment
    && Array.isArray(modelNode.source_);
}

module.exports = {
  isObjectNode: isObjectNode,
  hasArrayArguments: hasArrayArguments,
  hasType: hasType,
  hasPatternAlignment: hasPatternAlignment
};
