var peg = require("pegjs");
var fs = require('fs');
const _ = require('lodash');

///////////////////////////////////////////////////////////////////////////////
// from https://stackoverflow.com/questions/286141/remove-blank-attributes-from-an-object-in-javascript/

const removeEmpty = e => e instanceof Object ? Object.entries(e).reduce((o, [k, v]) => {
  if (typeof v === 'boolean' || v) o[k] = removeEmpty(v);
  return o;
}, e instanceof Array ? [] : {}) : e;

///////////////////////////////////////////////////////////////////////////////

Evaluator = function()
{
	var buf = fs.readFileSync( __dirname + '/../grammar.txt');
  this.parser = peg.generate(buf.toString());
}

Evaluator.prototype.evaluate = function(s)
{
	// Parses the command and returns a recursive tree of node stub
  const raw = this.parser.parse(s);
  return removeEmpty(raw);
}
