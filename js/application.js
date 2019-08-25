require('./input-evaluator.js');
require('./rendering-tree.js');
require('./playback/engine.js');
require('./playback/playback-device.js');

var Application = function()
{
	this.evaluator_ = new Evaluator();
  this.renderingTreeBuilder_ = new RenderingTreeBuilder();
	this.engine_ = new Engine();
	this.engine_.start();
	this.engine_.connect(this);
  this.playbackDevice_ = new GMDevice();
//  this.playbackDevice_ = new GMDevice('Microsoft GS Wavetable Synth 0');
//  this.playbackDevice_ = new VCVDevice('loopMIDI Port 1');
}

Application.prototype.parse = function(command)
{
	var result = this.evaluator_.evaluate(command);
  var renderingTree = this.renderingTreeBuilder_.rebuild(result);
	this.engine_.setRenderingTree(renderingTree);
	return JSON.stringify(result, undefined, 1);
};

Application.prototype.tick = function(event)
{
  this.playbackDevice_.tick(event);
}

module.exports = new Application();
