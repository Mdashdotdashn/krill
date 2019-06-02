var peg = require("pegjs");
var fs = require('fs');
var math = require('mathjs');

require("./operators/operators.js");

///////////////////////////////////////////////////////////////////////////////

function buildCommandTree(node)
{
	var child = buildOperationTree(node.sequence_);
	switch (node.name_)
	{
		case "slow":
			return makeSlowOperator(child, math.fraction(node.args_.amount));
	}
	throw "Unknown command: " + node.name_;
}


function buildOperationTree(node)
{
	// is the current node a sequence or operant ?
	switch(node.type_)
	{
		case "sequence":
			var sequence = new Sequence();
			sequence.renderArray(node);
			return sequence;
		case "command":
			return buildCommandTree(node);
		default:
			throw new Exception("unkonw node type:" + node.type_);
	}
}

// Evaluator
//
// Takes a string reprenting a command and transforms it into a tree of
// processing nodes
//

Evaluator = function()
{
	var buf = fs.readFileSync( __dirname + '/../grammar.txt');
  this.parser = peg.generate(buf.toString());
}

Evaluator.prototype.evaluate = function(s)
{
	// Parses the command and returns a recursive tree of nodes
  var result = this.parser.parse(s);
	// transforms the data nodes into an operation tree

  var tree = buildOperationTree(result);
	return tree.render();
}
