var math = require("mathjs");
var TimeUtils = require("../../utils/time-utils.js");

// Base class for render nodes that follow the common query pattern.
// Provides template method for input validation and fragment construction.
//
// Subclasses must implement:
//   - executeQuery_(start, end): Returns fragment array after validation
//
// Optional overrides:
//   - spanLength(): Returns the duration of this node's output
//
BaseQueryRenderNode = function()
{
}

// Template method: handles common validation and delegates to subclass logic.
BaseQueryRenderNode.prototype.query = function(start, end)
{
  var requestStart = TimeUtils.toFraction(start);
  var requestEnd = TimeUtils.toFraction(end);

  // Early exit: zero-width query window
  if (TimeUtils.equal(requestStart, requestEnd))
  {
    return [];
  }

  // Delegate to subclass-specific query logic
  var results = this.executeQuery_(requestStart, requestEnd);
  return results || [];
}

// Subclasses override this to implement operator-specific logic.
// Must return an array of QueryFragment objects (or null/undefined for empty result).
BaseQueryRenderNode.prototype.executeQuery_ = function(start, end)
{
  throw new Error("Subclass must implement executeQuery_");
}

// Optional: subclasses can override to provide duration information.
BaseQueryRenderNode.prototype.spanLength = function()
{
  return TimeUtils.toFraction(1);  // Default: one cycle
}

// Helper: check if a child exists and has a query method.
BaseQueryRenderNode.prototype.hasValidChild_ = function(child)
{
  return child && typeof child.query === 'function';
}

// Helper: construct a fragment object with all required fields.
BaseQueryRenderNode.prototype.makeFragment_ = function(wholeStart, wholeEnd, partStart, partEnd, value)
{
  return {
    wholeStart: TimeUtils.toFraction(wholeStart),
    wholeEnd: TimeUtils.toFraction(wholeEnd),
    partStart: TimeUtils.toFraction(partStart),
    partEnd: TimeUtils.toFraction(partEnd),
    value: String(value)
  };
}

// Helper: transform all fragments from a query by scaling their bounds.
BaseQueryRenderNode.prototype.scaleFragments_ = function(fragments, scaleFactor)
{
  var self = this;
  return (fragments || []).map(function(f) {
    return self.makeFragment_(
      TimeUtils.multiply(f.wholeStart, scaleFactor),
      TimeUtils.multiply(f.wholeEnd, scaleFactor),
      TimeUtils.multiply(f.partStart, scaleFactor),
      TimeUtils.multiply(f.partEnd, scaleFactor),
      f.value
    );
  });
}

// Helper: shift all fragments by a time offset.
BaseQueryRenderNode.prototype.shiftFragments_ = function(fragments, offset)
{
  var self = this;
  return (fragments || []).map(function(f) {
    return self.makeFragment_(
      TimeUtils.add(f.wholeStart, offset),
      TimeUtils.add(f.wholeEnd, offset),
      TimeUtils.add(f.partStart, offset),
      TimeUtils.add(f.partEnd, offset),
      f.value
    );
  });
}

// Helper: filter fragments to only those with valid wholeStart.
BaseQueryRenderNode.prototype.filterValidFragments_ = function(fragments)
{
  return (fragments || []).filter(function(f) {
    return f && f.wholeStart !== undefined;
  });
}

if (typeof module !== 'undefined' && module.exports)
{
  module.exports = BaseQueryRenderNode;
}
