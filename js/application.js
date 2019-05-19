require('./evaluator.js');

var Application = function()
{

}

Application.prototype.parse = function(command)
{
	var result this.evaluator_.evaluate(command);
	return JSON.stringify(result, undefined, 1);
};

module.exports = new Application();
