var TimeUtils = require("../../utils/time-utils.js");
require("./base-query-render-node.js");

VerticalPatternRenderNode = function(children)
{
  BaseQueryRenderNode.call(this);
  this.children_ = children || [];
}

VerticalPatternRenderNode.prototype = Object.create(BaseQueryRenderNode.prototype);
VerticalPatternRenderNode.prototype.constructor = VerticalPatternRenderNode;

VerticalPatternRenderNode.prototype.executeQuery_ = function(requestStart, requestEnd)
{
  var fragments = [];
  for (var i = 0; i < this.children_.length; i++)
  {
    var child = this.children_[i];
    if (!child || !(child.query instanceof Function))
    {
      continue;
    }

    var childFragments = child.query(requestStart, requestEnd) || [];
    for (var j = 0; j < childFragments.length; j++)
    {
      fragments.push(childFragments[j]);
    }
  }

  return fragments;
}
