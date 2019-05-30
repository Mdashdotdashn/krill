var assert = require("assert");
var math = require("mathjs");
require("../js/evaluator.js")

var evaluator = new Evaluator();
var quote ="\"";

var testEvaluator = function(s, expected,cycleLength)
{
  var sequence = evaluator.evaluate(s);
  var output = sequence.sequence_.reduce(function(a,x) { var o = new Object() ; o[x.timeString()] = x.values(); a.push(o); return a;}, []);
  assert.deepEqual(output,expected);
  assert.equal(math.format(sequence.cycleLength_), cycleLength);
}

testEvaluator("slow 2 $  "+quote+"1 2 3"+quote,
 [
   { "0/1" : ["1"] },
   { "2/3" : ["2"] },
   { "4/3" : ["3"] },
 ],
"2/1");


//console.log(evaluator.evaluate("slow 2 $ \"1 2 3\""));
//console.log(evaluator.evaluate("slow 0.5 $ slow 2 $ \"1 2 3\""));
