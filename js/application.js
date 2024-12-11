require('./input-evaluator.js');
require('./rendering-tree.js');
require('./playback/engine.js');
require('./playback/playback-device.js');
require('./playback/sync-device.js');

var Application = function()
{
	this.evaluator_ = new Evaluator();
  this.renderingTreeBuilder_ = new RenderingTreeBuilder();
	this.engine_ = new Engine();
//  this.playbackDevice_ = new GMDevice('Microsoft GS Wavetable Synth 0');
//  this.playbackDevice_ = new VCVDevice('loopMIDI Port 1');
}

Application.prototype.init = function(options)
{
  this.syncDevice_ = new SyncDevice(options.midiSync);
  this.engine_.start(this.syncDevice_);
	this.engine_.connect(this);
  this.playbackDevice_ = new GMDevice(options.midiDevice);
}

Application.prototype.parse = function(commandString)
{
	var result = this.evaluator_.evaluate(commandString);
  if (result.type_ == "command")
  {
    this.processCommand(result);
  }
  else
  {
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
      break;
    default:
      console.log("unknown command: " + JSON.stringify(command));
  }
}

Application.prototype.tick = function(event)
{
  this.playbackDevice_.tick(event);
}

module.exports = new Application();
