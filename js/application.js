require('./input-evaluator.js');
require('./rendering-tree.js');
require('./playback/engine.js');
require('./playback/playback-device.js');

var Application = function()
{
	this.evaluator_ = new Evaluator();
  this.renderingTree_ = new RenderingTree();
	this.engine_ = new Engine();
	this.engine_.start();
	this.engine_.connect(this);
  this.playbackDevice_ = new GMDevice('Microsoft GS Wavetable Synth 0');
//  this.playbackDevice_ = new VCVDevice('loopMIDI Port 1');
//  this.midiDevice_ = new easymidi.Output('loopMIDI Port 1');
//	this.midiDevice_ = new easymidi.Output('Microsoft GS Wavetable Synth 0');
//	this.midiDevice_ = new easymidi.Output('Teensy MIDI 2');
}

Application.prototype.parse = function(command)
{
	var result = this.evaluator_.evaluate(command);
  Dump(result);
  var renderingTree = this.renderingTree_.rebuild(result);
	this.engine_.setRenderingTree(renderingTree);
	return JSON.stringify(result, undefined, 1);
};

Application.prototype.tick = function(event)
{
  this.playbackDevice_.tick(event);
}

module.exports = new Application();
