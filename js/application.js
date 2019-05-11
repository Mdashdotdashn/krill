var peg = require("pegjs");
var fs = require('fs');
var sequence = require("./sequence.js");

var Application = function()
{
	console.log("loading grammar..");
	fs.readFile( __dirname + '/../grammar.txt', function (err, data) {
	  if (err) {
	    throw err;
	  }
		try {
			this.parser = peg.generate(data.toString());
		}
		catch(err)
		{
			console.log(err.message);
			console.log(err.location);
			throw err;
		}
		console.log("loaded.");
	});
}

Application.prototype.parse = function(command)
{
	var result = parser.parse(command); // returns something like a function call: method, arg1, arg2, arg3
	console.log("translating: " + JSON.stringify(result));
	var sequence = this.toSequence(result);
	console.log("result: " + JSON.stringify(sequence));
	return JSON.stringify(sequence);
};

Application.prototype.toSequence = function(sequenceArray)
{
	var sequence = new Sequence(sequenceArray);
	return sequence;
}

module.exports = new Application();
