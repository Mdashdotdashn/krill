require('./evaluator.js');
require('./playback/engine.js');

var Application = function()
{
	this.evaluator_ = new Evaluator();
	this.engine_ = new Engine();
	this.engine_.start();
	this.engine_.connect(this);
}

Application.prototype.parse = function(command)
{
	var result = this.evaluator_.evaluate(command);
	this.engine_.setSequence(result);
	return JSON.stringify(result, undefined, 1);
};

Application.prototype.tick = function(values)
{
	console.log("ticked with " + values);
}

module.exports = new Application();
