var math = require("mathjs");
require("../renderer/query-contract.js");
var TimeUtils = require("../utils/time-utils.js");

// Minimal query-only player used during teardown/rebuild.
// It preserves the player seam while delegating behavior to RenderTree.query().
//
// API CONTRACT:
// ==============
// PREFERRED (new, recommended):
//   - nextOnsetTimeFrom(time) → Fraction: Next onset strictly after time
//   - eventsAtTime(time) → string[]: Values at the given time, or [] if none
//
// COMPATIBILITY (deprecated, for migration period):
//   - advance(time) → Fraction: Alias for nextOnsetTimeFrom()
//   - eventForTime(time) → {time, values} | null: Alias for eventsAtTime() (wraps in object)
//   - eventsForTime(time) → string[]: Alias for eventsAtTime()
//
// See: docs/standalone-app-restore-plan.md Phase 4
//
RenderingTreePlayer = function()
{
  this.renderingTree_ = null;
  this.pendingRenderingTree_ = null;
}

RenderingTreePlayer.prototype.setRenderingTree = function(tree)
{
  if (!this.renderingTree_)
  {
    this.renderingTree_ = tree;
    return;
  }

  // Mid-cycle updates are applied when nextOnsetTimeFrom() crosses a cycle boundary.
  this.pendingRenderingTree_ = tree;
}

// Alias kept for naming parity with C++ RenderTreePlayer::setTree.
RenderingTreePlayer.prototype.setTree = function(tree)
{
  this.setRenderingTree(tree);
}

RenderingTreePlayer.prototype.reset = function()
{
  if (this.pendingRenderingTree_)
  {
    this.renderingTree_ = this.pendingRenderingTree_;
    this.pendingRenderingTree_ = null;
  }
}

RenderingTreePlayer.prototype.toFraction_ = TimeUtils.toFraction;
RenderingTreePlayer.prototype.cycleStart_ = TimeUtils.cycleStart;
RenderingTreePlayer.prototype.nextCycleBoundary_ = TimeUtils.nextCycleBoundary;
RenderingTreePlayer.prototype.epsilon_ = TimeUtils.epsilon;

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
  var epsilon = this.epsilon_();
  var end = math.add(start, epsilon);
  return this.queryArc(start, end);
}

// Preferred payload API: returns the values array at the given time, or [] if none.
RenderingTreePlayer.prototype.eventsAtTime = function(time)
{
  var eventTime = this.toFraction_(time);
  var fragments = this.queryPointWindow(eventTime);
  var values = [];

  fragments.forEach(function(fragment) {
    if (fragment.wholeStart === undefined)
    {
      return;
    }

    if (TimeUtils.equal(fragment.wholeStart, eventTime))
    {
      values.push(String(fragment.value));
    }
  });

  return values;
}

// Preferred scheduler API: returns the next onset time strictly after the given time.
//
// Uses a point query (queryPointWindow) at each step rather than a large arc
// query, so time-varying render-node parameters (e.g. ShiftRenderNode amounts)
// are always resolved at the correct point in time. Within each step, wholeEnd
// from the returned fragment is used to jump directly to the start of the next
// slot, avoiding fixed-size step scanning.
RenderingTreePlayer.prototype.nextOnsetTimeFrom = function(time)
{
  var current = this.toFraction_(time);
  var nextBoundary = this.nextCycleBoundary_(current);
  var lookAheadCycles = math.fraction(16);
  var searchEnd = this.pendingRenderingTree_
    ? nextBoundary
    : math.add(current, lookAheadCycles);

  var t = current;

  while (TimeUtils.smaller(t, searchEnd))
  {
    var fragments = this.queryPointWindow(t);

    if (fragments.length === 0)
    {
      t = this.nextCycleBoundary_(t);
      continue;
    }

    var nextOnset = null;
    var nextT = null;

    for (var i = 0; i < fragments.length; i++)
    {
      var f = fragments[i];
      if (f.wholeStart === undefined)
      {
        continue;
      }

      var onset = TimeUtils.toFraction(f.wholeStart);

      // Onset strictly after current and within the search range.
      if (TimeUtils.larger(onset, current) && TimeUtils.smallerEq(onset, searchEnd))
      {
        if (nextOnset === null || TimeUtils.smaller(onset, nextOnset))
        {
          nextOnset = onset;
        }
      }

      // wholeEnd gives the exact start of the next slot — use it to advance t.
      if (f.wholeEnd !== undefined)
      {
        var end = TimeUtils.toFraction(f.wholeEnd);
        if (TimeUtils.larger(end, t) && (nextT === null || TimeUtils.smaller(end, nextT)))
        {
          nextT = end;
        }
      }
    }

    if (nextOnset !== null)
    {
      return nextOnset;
    }

    t = nextT !== null ? nextT : this.nextCycleBoundary_(t);
  }

  if (this.pendingRenderingTree_)
  {
    this.renderingTree_ = this.pendingRenderingTree_;
    this.pendingRenderingTree_ = null;
  }

  return nextBoundary;
}

// Compatibility aliases for migration period (deprecated, use preferred API above).
RenderingTreePlayer.prototype.advance = function(time)
{
  return this.nextOnsetTimeFrom(time);
}

RenderingTreePlayer.prototype.eventForTime = function(time)
{
  var values = this.eventsAtTime(time);
  if (values && values.length > 0)
  {
    return {time: this.toFraction_(time), values: values};
  }
  return null;
}

RenderingTreePlayer.prototype.eventsForTime = function(time)
{
  return this.eventsAtTime(time);
}


