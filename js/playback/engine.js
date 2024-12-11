var NanoTimer = require("nanotimer");
var EventEmitter = require('events').EventEmitter;
var math = require('mathjs');
require("./rendering-tree-player.js");
var util = require('util');

Engine = function()
{
  this.timer_ = new NanoTimer();
  this.player_ = new RenderingTreePlayer();
  this.cps_ = 1;
}

util.inherits(Engine, EventEmitter);

Engine.prototype.connect = function(target)
{
  if (target instanceof Function)
  {
    this.on("tick", target);
  }
  else
  {
    this.on("tick", function(tickCount)
      {
        target.tick(tickCount);
      });
  }
}

// sets the base tempo as a number of 
// cycles per seconds.
Engine.prototype.setCps = function(cps)
{
  if (!this.synced_)
  {
    this.cps_ = cps;    
  }
}

// clear the engine's output
Engine.prototype.hush = function()
{
  this.player_.clear();
}

Engine.prototype.start = function(syncDevice)
{
  this.cps_ = 0.5; // In the very beginning we don't know our tempo
  // resets the internal time & clear the player
  this.nextEventTime_ = math.fraction(0,1);
  this.player_.clear();

  // Are we synced ?
  this.synced_ = syncDevice.enabled();
  if (this.synced_)
  {
    console.log("*running synced*");
    syncDevice.connect(this);
  }
  else
  {
    // triggers the processing for time 0
    this.processUnsyncedEvent();    
  }
}

Engine.prototype.onSyncStart = function()
{
  console.log("start");
  this.syncOn_ = true;
  this.time1_ = null;
  this.time2_ = null;
  this.cycleTime_ = math.fraction(-1,24);
  this.nextEventTime_ = math.fraction(0);
  if (this.renderingTree_)
  {
    this.player_.setRenderingTree(this.renderingTree_);
  }
  this.processSyncedEvent();
}

Engine.prototype.onSyncStop = function()
{
  console.log("stop");
  this.syncOn_ = false;
  this.player_.clear();
}

Engine.prototype.onSyncClock = function()
{
  if (this.syncOn_)
  {
    const clockLength = math.fraction(1, 24);
    this.time1_ = this.time2_;
    this.time2_ = Date.now();
    this.cycleTime_ = math.add(this.cycleTime_, clockLength);


    // Sometime we can get time1_ == time2_ and I don't know why
    if (this.time1_ != null && (this.time1_ != this.time2_))
    {
      this.cps_ = (math.number(clockLength)) / (this.time2_ - this.time1_) / 1e-3;

      // Since we now have a new precise time reference, we update the timer
      // to the new event
      this.timer_.clearTimeout();

      // If we are ahead of the next event to play. play it directly
      if (math.compare(this.cycleTime_, this.nextEventTime_) >= 0)
      {
        this.processPlayerEvent();
      }

      // Recalibrate our timer to this new time base
      var offsetToNextEvent = math.number(math.subtract(this.nextEventTime_ , this.cycleTime_)) / this.cps_ * 1000;
      this.timer_.setTimeout(function(engine) { engine.processSyncedEvent();} , [this], "" + offsetToNextEvent +"m");
    }
  }
}

Engine.prototype.processUnsyncedEvent = function()
{
  this.timer_.clearTimeout();
  var offsetToNextEvent = this.processPlayerEvent();
  this.timer_.setTimeout(function(engine) { engine.processUnsyncedEvent();} , [this], "" + offsetToNextEvent +"m");
}

Engine.prototype.processSyncedEvent = function()
{
  this.timer_.clearTimeout();
  console.log("processing callback event at " + this.nextEventTime_);
  var offsetToNextEvent = this.processPlayerEvent();
  console.log("offset " + offsetToNextEvent + "ms");
  this.timer_.setTimeout(function(engine) { engine.processSyncedEvent();} , [this], "" + offsetToNextEvent +"m");
//  console.log("out");
}

// process the event we're at. Most of the time it relates
// to an event where the player has some events send
// but it could also be triggered by 'manually' like
// when the engine starts for example
Engine.prototype.processPlayerEvent = function()
{
  // See if the player has some events and broadcast them
  var eventTime = this.nextEventTime_;
  var event = this.player_.eventForTime(eventTime);
  if (event) this.emit("tick", event);

  // Ask the player what is the next cycle time it has event for
  var nextEventTime = this.player_.advance(eventTime);
  // Translate the fractional cycle time position to a time offset
  // from the current time and set a timeout for the next tick.
  var offset = math.number(math.subtract(nextEventTime,eventTime)) / this.cps_ * 1000;

  // Store the next cycle time
  this.nextEventTime_ = nextEventTime;

  // return the number of milliseconds before the next event
  return offset;
}

Engine.prototype.setRenderingTree = function(s)
{
  this.renderingTree_ = s;
  this.player_.setRenderingTree(s);
}
