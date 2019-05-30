require("../sequence.js");
var math = require("mathjs");

SequencePlayer = function()
{
  // Current sequence
  this.sequence_ = undefined;
  // Next queued sequence
  this.queued_ = undefined;
  // Specify we're heading toward a cycle reset (no sequence/end of sequence)
  this.cycleReset_ = false;
}

SequencePlayer.prototype.setSequence = function(sequence)
{
  this.queued_ = sequence;
}

SequencePlayer.prototype.advance = function(time)
{
  var f = math.fraction(time);
  var cycleLength = math.fraction(this.sequence_ ? this.sequence_.cycleLength_ : "1/1");
  var cycleTime = math.mod(f,cycleLength);
  var offset = math.floor(math.divide(f, cycleLength));

//  console.log("-----------------------------"+ time);
//  console.log("length = "+ cycleLength);
//  console.log("inner cycle time = "+cycleTime);
//  console.log("cycle offset = "+offset);

  // If we don't have a sequence, we reply we should be triggered
  // at next cycle. We also flag resetCycle so that we queue an incoming
  // sequence if queued
  if (!this.sequence_)
  {
    var position = math.add(offset, cycleLength);
    this.current_ = new Step(position,undefined);
    this.resetCycle_ = true;
  }
  else {
    var nextData = this.sequence_.nextTimeFrom(cycleTime);
    if (!nextData) this.resetCycle_ = true;
    var position = fracToString(math.add(offset, nextData ? nextData.time() : cycleLength));
    this.current_ = new Step(position, nextData ? nextData.values() : undefined);
  }
  return this.current_.time();
}

SequencePlayer.prototype.currentValues = function()
{
  // If the last advance lead to a cycle end we evaluate possible
  // queueing and set data to the start of the next cycle

  if (this.resetCycle_)
  {
    if (this.queued_ && this.queued_.size() > 0)
    {
      // trigger the sequence and update the current values
      // to be the data at the beginning of it.
      this.sequence_ = this.queued_;
    }
    // This is a shortcut not taking into account there could be no data at
    // for 0/1 (for example "rotL 0.01 $ [1, 2]")
    this.current_.values_ = this.sequence_.dataAtIndex(0).values_;

    this.queued_ = undefined;
    this.resetCycle_ = false;
  }

  // return the value
  return this.current_.values();
}
