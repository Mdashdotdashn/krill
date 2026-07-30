require("../nodes/horizontal-pattern-render-node.js");
require("../nodes/timeline-pattern-render-node.js");
require("../nodes/vertical-pattern-render-node.js");
var factoryUtils = require("./factory-utils.js");

function makeHorizontalPatternNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasPatternAlignment(modelNode, "h"))
  {
    return null;
  }

  var children = modelNode.source_.map(function(child)
  {
    return buildRenderNode(child);
  });

  var weights = modelNode.source_.map(function(child)
  {
    if (!factoryUtils.isObjectNode(child) || !child.options_ || child.options_.weight === undefined)
    {
      return 1;
    }
    return child.options_.weight;
  });

  return HorizontalPatternRenderNode.withWeights(children, weights);
}

function makeVerticalPatternNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasPatternAlignment(modelNode, "v"))
  {
    return null;
  }

  var children = modelNode.source_.map(function(child)
  {
    return buildRenderNode(child);
  });
  return new VerticalPatternRenderNode(children);
}

function makeTimelinePatternNode(modelNode, buildRenderNode)
{
  if (!factoryUtils.hasPatternAlignment(modelNode, "t"))
  {
    return null;
  }

  var children = modelNode.source_.map(function(child)
  {
    return buildRenderNode(child);
  });
  return new TimelinePatternRenderNode(children);
}

module.exports = {
  makeHorizontalPatternNode: makeHorizontalPatternNode,
  makeVerticalPatternNode: makeVerticalPatternNode,
  makeTimelinePatternNode: makeTimelinePatternNode
};
