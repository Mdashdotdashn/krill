var peg = require("pegjs");
var fs = require('fs');

require("./reducer.js");

Evaluator = function()
{
	var buf = fs.readFileSync( __dirname + '/../grammar.txt');
  this.parser = peg.generate(buf.toString());

  this.reducer = new Reducer();
}

Evaluator.prototype.evaluate = function(s)
{
  var result = this.parser.parse(s); // returns something like a function call: method, arg1, arg2, arg3
  return this.reducer.reduce(result);
}
