var peg = require("pegjs");
var fs = require('fs');

///////////////////////////////////////////////////////////////////////////////

Evaluator = function()
{
	var buf = fs.readFileSync( __dirname + '/../grammar.txt');
  this.parser = peg.generate(buf.toString());
}

Evaluator.prototype.evaluate = function(s)
{
	// Parses the command and returns a recursive tree of node stub
  return this.parser.parse(s);
}
