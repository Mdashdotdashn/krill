var math = require("mathjs");
require("../renderer/query-contract.js");

// Minimal query-only player used during teardown/rebuild.
// It preserves the player seam while delegating behavior to RenderTree.query().
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

  // Mid-cycle updates are applied when advance() crosses a cycle boundary.
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

RenderingTreePlayer.prototype.toFraction_ = function(value)
{
  return math.fraction(value);
}

RenderingTreePlayer.prototype.cycleStart_ = function(time)
{
  return math.fraction(math.floor(this.toFraction_(time)));
}

RenderingTreePlayer.prototype.nextCycleBoundary_ = function(time)
{
  return math.add(this.cycleStart_(time), math.fraction(1));
}

RenderingTreePlayer.prototype.epsilon_ = function()
{
  return math.fraction(1, 1024);
}

RenderingTreePlayer.prototype.uniqueOnsetsInWindow_ = function(startExclusive, end, includeEnd)
{
  var fragments = this.queryArc(startExclusive, end);
  var onsets = [];

  fragments.forEach(function(fragment) {
    if (fragment.wholeStart === undefined)
    {
      return;
    }

    var onset = math.fraction(fragment.wholeStart);
    if (!math.larger(onset, startExclusive))
    {
      return;
    }

    if (includeEnd)
    {
      if (math.larger(onset, end))
      {
        return;
      }
    }
    else if (!math.smaller(onset, end))
    {
      return;
    }

    var exists = onsets.some(function(t) {
      return math.equal(t, onset);
    });

    if (!exists)
    {
      onsets.push(onset);
    }
  });

  onsets.sort(function(a, b) {
    return math.compare(a, b);
  });

  return onsets;
}

RenderingTreePlayer.prototype.nextWholeEndAfter_ = function(time)
{
  var t = this.toFraction_(time);
  var fragments = this.queryPointWindow(t);
  var candidate = null;

  fragments.forEach(function(fragment) {
    if (fragment.wholeEnd === undefined)
    {
      return;
    }

    var end = math.fraction(fragment.wholeEnd);
    if (!math.larger(end, t))
    {
      return;
    }

    if (!candidate || math.smaller(end, candidate))
    {
      candidate = end;
    }
  });

  return candidate;
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
  var epsilon = this.epsilon_();
  var end = math.add(start, epsilon);
  return this.queryArc(start, end);
}

RenderingTreePlayer.prototype.eventForTime = function(time)
{
  var eventTime = this.toFraction_(time);
  var fragments = this.queryPointWindow(eventTime);
  var values = [];

  fragments.forEach(function(fragment) {
    if (fragment.wholeStart === undefined)
    {
      return;
    }

    if (math.equal(math.fraction(fragment.wholeStart), eventTime))
    {
      values.push(String(fragment.value));
    }
  });

  if (values.length === 0)
  {
    return null;
  }

  return {
    time: eventTime,
    values: values
  };
}

RenderingTreePlayer.prototype.eventsForTime = function(time)
{
  var event = this.eventForTime(time);
  return event ? event.values : [];
}

RenderingTreePlayer.prototype.advance = function(time)
{
  var current = this.toFraction_(time);
  var nextBoundary = this.nextCycleBoundary_(current);
  var epsilon = this.epsilon_();
  var searchStep = math.fraction(1, 3072);

  var firstEventInRange = function(player, startExclusive, endInclusive) {
    var t = math.add(startExclusive, searchStep);
    while (math.smallerEq(t, endInclusive))
    {
      var event = player.eventForTime(t);
      if (event && event.values && event.values.length > 0)
      {
        return t;
      }
      t = math.add(t, searchStep);
    }
    return null;
  };

  if (this.pendingRenderingTree_)
  {
    var beforeBoundary = firstEventInRange(this, current, math.subtract(nextBoundary, epsilon));
    if (beforeBoundary)
    {
      return beforeBoundary;
    }

    this.renderingTree_ = this.pendingRenderingTree_;
    this.pendingRenderingTree_ = null;
    return nextBoundary;
  }

  var lookAheadCycles = math.fraction(16);
  var nextEvent = firstEventInRange(this, current, math.add(current, lookAheadCycles));
  if (nextEvent)
  {
    return nextEvent;
  }

  return nextBoundary;
}
