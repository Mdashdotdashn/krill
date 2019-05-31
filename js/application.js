require('./evaluator.js');
require('./playback/engine.js');
require('./midi/midi.js');

var easymidi = require('easymidi');

console.log(easymidi.getOutputs());

var Application = function()
{
	this.evaluator_ = new Evaluator();
	this.engine_ = new Engine();
	this.engine_.start();
	this.engine_.connect(this);
	this.midiDevice_ = new easymidi.Output('Microsoft GS Wavetable Synth 0');
	this.values_ = undefined;
}

Application.prototype.parse = function(command)
{
	var result = this.evaluator_.evaluate(command);
	this.engine_.setSequence(result);
	return JSON.stringify(result, undefined, 1);
};

Application.prototype.tick = function(values)
{
	var device = this.midiDevice_;

	processNotes = function(v,m)
	{
		if (v) v.forEach(function(x) {
			var note = parseInt(x) + 48;
			console.log(note);
			device.send(m, {
			  note: note,
			  velocity: 127,
			  channel: 0
			});
		});
	}

	processNotes(this.values_, 'noteon');
	this.values_ = values;
	processNotes(this.values_, 'noteoff');
}

module.exports = new Application();
