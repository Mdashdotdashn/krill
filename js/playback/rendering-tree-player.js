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

    if (math.equal(math.fraction(fragment.wholeStart), eventTime))
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

  while (math.smaller(t, searchEnd))
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

      var onset = math.fraction(f.wholeStart);

      // Onset strictly after current and within the search range.
      if (math.larger(onset, current) && math.smallerEq(onset, searchEnd))
      {
        if (nextOnset === null || math.smaller(onset, nextOnset))
        {
          nextOnset = onset;
        }
      }

      // wholeEnd gives the exact start of the next slot — use it to advance t.
      if (f.wholeEnd !== undefined)
      {
        var end = math.fraction(f.wholeEnd);
        if (math.larger(end, t) && (nextT === null || math.smaller(end, nextT)))
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


