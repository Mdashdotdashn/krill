var math = require("mathjs");

// Minimal query-only player used during teardown/rebuild.
// It preserves the player seam while delegating behavior to RenderTree.query().
RenderingTreePlayer = function()
{
  this.renderingTree_ = null;
}

RenderingTreePlayer.prototype.setRenderingTree = function(tree)
{
  this.renderingTree_ = tree;
}

RenderingTreePlayer.prototype.queryArc = function(start, end)
{
  if (!this.renderingTree_ || !this.renderingTree_.query)
  {
    return [];
  }
  return this.renderingTree_.query(start, end) || [];
}

RenderingTreePlayer.prototype.queryPointWindow = function(time)
{
  var start = math.fraction(time);
  var epsilon = math.fraction(1, 1024);
  var end = math.add(start, epsilon);
  return this.queryArc(start, end);
}
