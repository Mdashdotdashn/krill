var EventEmitter = require('events').EventEmitter;
var util = require('util');
var math = require('mathjs');

require('./rendering-tree-player.js');

Engine = function()
{
  this.cps_ = 1;
  this.renderingTree_ = null;
  this.renderingPlayer_ = new RenderingTreePlayer();
  this.running_ = false;
  this.syncOn_ = false;
  this.syncedClockCount_ = 0;
  this.clocksPerCycle_ = 96;
  this.currentTime_ = math.fraction(0);
  this.unsyncedTimer_ = null;
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

Engine.prototype.setCps = function(cps)
{
  this.cps_ = cps;
}

Engine.prototype.hush = function()
{
  this.running_ = false;
  this.syncOn_ = false;
  if (this.unsyncedTimer_)
  {
    clearTimeout(this.unsyncedTimer_);
    this.unsyncedTimer_ = null;
  }
  this.currentTime_ = math.fraction(0);
  this.renderingPlayer_.reset();
}

Engine.prototype.start = function(syncDevice)
{
  this.running_ = true;
  this.synced_ = syncDevice.enabled();

  syncDevice.connect(this);

  if (!this.synced_)
  {
    this.syncOn_ = false;
    this.currentTime_ = math.fraction(0);
    this.renderingPlayer_.reset();
    this.processUnsyncedEvent();
  }
}

Engine.prototype.onSyncStart = function()
{
  this.running_ = true;
  this.syncOn_ = true;
  this.syncedClockCount_ = 0;
  this.currentTime_ = math.fraction(0);
  this.renderingPlayer_.reset();
  this.processSyncedEvent();
}

Engine.prototype.onSyncStop = function()
{
  this.syncOn_ = false;
  this.running_ = false;
  this.syncedClockCount_ = 0;
}

Engine.prototype.onSyncClock = function()
{
  if (!this.running_ || !this.syncOn_)
  {
    return;
  }

  this.syncedClockCount_ += 1;
  this.currentTime_ = math.fraction(this.syncedClockCount_, this.clocksPerCycle_);
  this.processSyncedEvent();
}

Engine.prototype.processUnsyncedEvent = function()
{
  if (!this.running_)
  {
    return;
  }

  var values = this.renderingPlayer_.eventsAtTime(this.currentTime_);
  if (values && values.length > 0)
  {
    this.emit("tick", {time: this.currentTime_, values: values});
  }

  var nextTime = this.renderingPlayer_.nextOnsetTimeFrom(this.currentTime_);
  var deltaCycles = math.subtract(nextTime, this.currentTime_);
  var delayMs = Math.max(1, Math.round((math.number(deltaCycles) * 1000) / this.cps_));
  this.currentTime_ = nextTime;

  var self = this;
  this.unsyncedTimer_ = setTimeout(function() {
    self.processUnsyncedEvent();
  }, delayMs);
}

Engine.prototype.processSyncedEvent = function()
{
  if (!this.running_ || !this.syncOn_)
  {
    return;
  }

  this.processPlayerEvent();
}

Engine.prototype.processPlayerEvent = function()
{
  var event = this.renderingPlayer_.eventForTime(this.currentTime_);
  if (event && event.values && event.values.length > 0)
  {
    this.emit("tick", event);
  }
  return event ? event.values.length : 0;
}

Engine.prototype.setRenderingTree = function(tree)
{
  this.renderingTree_ = tree;
  this.renderingPlayer_.setRenderingTree(tree);
}
