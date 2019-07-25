require("../pattern.js");
var math = require("mathjs");

RenderingTreePlayer = function()
{
  // current rendering tree
  this.renderingTree_ = undefined;
  // Next queued sequence
  this.queued_ = undefined;
  // Specify we're heading toward a cycle reset (no sequence/end of sequence)
  this.cycleReset_ = false;
  // Current sequence
  this.sequence_ = undefined;
  // Local cycle offset
  this.cycleOffset_ = math.fraction(0);
}

RenderingTreePlayer.prototype.setRenderingTree = function(tree)
{
  this.queued_ = tree;
}

RenderingTreePlayer.prototype.cycleLength = function()
{
  return math.fraction(this.sequence_ ? this.sequence_.cycleLength_ : "1/1");
}

RenderingTreePlayer.prototype.cycleTimeAndStart = function(time)
{
  var localTime = math.subtract(math.fraction(time), this.cycleOffset_);
  var cycleLength = this.cycleLength();
  var cycleTime = math.mod(localTime,cycleLength);
  var cycleStart = math.multiply(math.floor(math.divide(localTime, cycleLength)), cycleLength) + this.cycleOffset_;
  return { cycleTime: cycleTime, start: cycleStart};
}

RenderingTreePlayer.prototype.advance = function(time)
{
//  console.log("-----------------------------"+ time);
//  console.log("length = "+ cycleLength);
//  console.log("inner cycle time = "+cycleTime);
//  console.log("cycle offset = "+offset);

  const cycleTimeAndStart = this.cycleTimeAndStart(time);
  const cycleStart = cycleTimeAndStart.start;
  const cycleTime = cycleTimeAndStart.cycleTime;
  const cycleLength = this.cycleLength();

  // If we don't have a sequence, we reply we should be triggered
  // at next cycle. We also flag resetCycle so that we queue an incoming
  // sequence if queued
  if (!this.sequence_)
  {
    var position = math.add(cycleStart, cycleLength);
    this.current_ = new Event(position,undefined);
    this.resetCycle_ = true;
  }
  else {
    var nextData = this.sequence_.nextTimeFrom(cycleTime);
    if (!nextData) this.resetCycle_ = true;
    var position = fracToString(math.add(cycleStart, nextData ? nextData.time() : cycleLength));
    this.current_ = new Event(position, nextData ? nextData.values() : undefined);
  }
  return this.current_.time();
}

RenderingTreePlayer.prototype.reset = function()
{
  // probably a lot more complex than this
  this.resetCycle_ = true;
  this.sequence_ = undefined;
  this.queued_ = undefined;
  this.current_ = new Event("0", null);
}

// Returns the event queued for the current time if it matches the time

RenderingTreePlayer.prototype.eventForTime = function(currentTime)
{
  // If the last advance lead to a cycle end we evaluate possible
  // queueing and set data to the start of the next cycle
  if (this.resetCycle_)
  {
    if (this.queued_)
    {
      // trigger the sequence and update the current values
      // to be the data at the beginning of it.
      this.renderingTree_ = this.queued_;
    }
    // This is a shortcut not taking into account there could be no data at
    // for 0/1 (for example "rotL 0.01 $ [1, 2]")

    if (this.renderingTree_)
    {
      this.sequence_ = this.renderingTree_.render();
      this.cycleOffset_ = currentTime;
      var firstSlice = this.sequence_.dataAtIndex(0);
      this.current_.values_ = math.equal(firstSlice.time_, math.fraction(0)) ? firstSlice.values_ : null;
      this.renderingTree_.tick();
    }
    else
    {
      this.current_.values_ = null;
    }
    this.queued_ = undefined;
    this.resetCycle_ = false;
  }

  // Look if an event is set for this player
  if (this.current_ && this.current_.values_)
  {
    // Look if it matches the current time
    if (math.equal(math.fraction(currentTime), this.current_.time()))
    {
      var targetName = this.sequence_ ? this.sequence_.targetName_: undefined;
      return { target: targetName, values: this.current_.values() };
    }
  }
}
