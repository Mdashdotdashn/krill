var TimeUtils = require("../../utils/time-utils.js");
require("./base-query-render-node.js");

EmptyRenderNode = function()
{
  BaseQueryRenderNode.call(this);
}

EmptyRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
EmptyRenderNode.prototype.constructor = EmptyRenderNode;

EmptyRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  return [];
}
