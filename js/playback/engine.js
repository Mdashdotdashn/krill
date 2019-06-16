var NanoTimer = require("nanotimer");
var EventEmitter = require('events').EventEmitter;
var math = require('mathjs');
require("./sequence-player.js");
var util = require('util');

Engine = function()
{
  this.timer_ = new NanoTimer();
  this.player_ = new SequencePlayer();
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

Engine.prototype.start = function()
{
  this.currentTime_ = math.fraction("0");
  this.player_.reset();
  this.processTick();
}

Engine.prototype.processTick = function()
{
  var currentCycleTime = this.currentTime_;
  var values = this.player_.eventForTime(currentCycleTime);
  if (values) this.emit("tick", values);
  var nextCycleTime = this.player_.advance(currentCycleTime);
  var offset = math.number(math.subtract(nextCycleTime,currentCycleTime)) / this.cps_ * 1000;
  this.timer_.setTimeout(function(engine) { engine.processTick();} , [this], "" + offset +"m");
  this.currentTime_ = nextCycleTime;
}

Engine.prototype.setRenderingTree = function(s)
{
  this.player_.setRenderingTree(s);
}
