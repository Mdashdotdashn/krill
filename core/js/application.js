require('./input-evaluator.js');
require('./renderer/render-tree.js');
require('./playback/engine.js');
require('./playback/playback-device.js');
require('./playback/sync-device.js');
require('./type.js');

var Application = function()
{
	this.evaluator_ = new Evaluator();
  this.renderingTreeBuilder_ = new RenderingTreeBuilder();
  this.engine_ = new Engine();
  this.reportedEvents_ = [];
  this.maxReportedEvents_ = 512;
//  this.playbackDevice_ = new GMDevice('Microsoft GS Wavetable Synth 0');
//  this.playbackDevice_ = new VCVDevice('loopMIDI Port 1');
}

Application.prototype.init = function(options)
{
  this.playbackDevice_ = new GMDevice(options.midiDevice);

  this.syncDevice_ = new SyncDevice(options.midiSync);
	this.engine_.connect(this);
  this.engine_.start(this.syncDevice_);

  if (options.cycle)
  {
    this.parse("'" + options.cycle + "'");
  }
}

Application.prototype.parse = function(commandString)
{
	var result = this.evaluator_.evaluate(commandString);
  if (result.type_ == "command")
  {
    if (typeof Dump === "function")
    {
      Dump(result);
    }
    this.processCommand(result);
  }
  else
  {
    var dumper = new Object();
    dumper["source"] = commandString;
    dumper["model"] = result;
    if (typeof DumpForCpp === "function")
    {
      DumpForCpp(dumper);
    }
    var renderingTree = this.renderingTreeBuilder_.rebuild(result);
  	this.engine_.setRenderingTree(renderingTree);
  	return JSON.stringify(result, undefined, 1);
  }
};

Application.prototype.processCommand = function(command)
{
  switch(command.name_)
  {
    case "setcps":
      this.engine_.setCps(parseFloat(command.options_.value));
      break;
    case "hush":
      this.engine_.hush();
      this.playbackDevice_.hush();
      this.clearReportedEvents();
      break;
    default:
      console.log("unknown command: " + JSON.stringify(command));
  }
}

Application.prototype.tick = function(event)
{
  this.playbackDevice_.tick(event);
  this.enqueueReportedEvent(event);
}

Application.prototype.enqueueReportedEvent = function(event)
{
  this.reportedEvents_.push({
    time: event && event.time !== undefined ? String(event.time) : null,
    values: event && Array.isArray(event.values) ? event.values.slice() : []
  });

  if (this.reportedEvents_.length > this.maxReportedEvents_)
  {
    this.reportedEvents_.splice(0, this.reportedEvents_.length - this.maxReportedEvents_);
  }
}

Application.prototype.drainReportedEvents = function()
{
  var snapshot = this.reportedEvents_.slice();
  this.reportedEvents_.length = 0;
  return snapshot;
}

Application.prototype.clearReportedEvents = function()
{
  this.reportedEvents_.length = 0;
}

module.exports = new Application();
