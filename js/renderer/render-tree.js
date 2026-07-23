// RenderTree compiles the model into query nodes once.
// Query paths delegate to the compiled node tree, not raw model inspection.

require("./nodes/empty-render-node.js");
require("./nodes/element-render-node.js");
require("./nodes/horizontal-pattern-render-node.js");

function buildRenderNode(modelNode)
{
  if (!modelNode || !(modelNode instanceof Object))
  {
    return new EmptyRenderNode();
  }

  if (modelNode.type_ === "element")
  {
    if (modelNode.source_ && modelNode.source_ instanceof Object)
    {
      return new ElementRenderNode(buildRenderNode(modelNode.source_));
    }
    return new ElementRenderNode(modelNode.source_);
  }

  if (modelNode.type_ === "pattern"
      && modelNode.arguments_
      && modelNode.arguments_.alignment === "h"
      && Array.isArray(modelNode.source_))
  {
    var children = modelNode.source_.map(function(child)
    {
      return buildRenderNode(child);
    });
    return new HorizontalPatternRenderNode(children);
  }

  return new EmptyRenderNode();
}

RenderTree = function(modelNodeTree)
{
  this.rootNode_ = buildRenderNode(modelNodeTree);
}

RenderTree.prototype.query = function(start, end)
{
  return this.rootNode_.query(start, end);
}

RenderTreeBuilder = function()
{
}

RenderTreeBuilder.prototype.rebuild = function(modelNodeTree)
{
  return new RenderTree(modelNodeTree);
}

// Backward compatibility with existing JS call sites.
RenderingTreeBuilder = RenderTreeBuilder;
