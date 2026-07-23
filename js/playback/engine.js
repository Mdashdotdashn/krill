var EventEmitter = require('events').EventEmitter;
var util = require('util');

Engine = function()
{
  this.cps_ = 1;
  this.renderingTree_ = null;
  this.running_ = false;
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
}

Engine.prototype.start = function(syncDevice)
{
  this.running_ = true;
  this.synced_ = syncDevice.enabled();
}

Engine.prototype.onSyncStart = function()
{
}

Engine.prototype.onSyncStop = function()
{
}

Engine.prototype.onSyncClock = function()
{
}

Engine.prototype.processUnsyncedEvent = function()
{
}

Engine.prototype.processSyncedEvent = function()
{
}

Engine.prototype.processPlayerEvent = function()
{
  return 0;
}

Engine.prototype.setRenderingTree = function(tree)
{
  this.renderingTree_ = tree;
}
