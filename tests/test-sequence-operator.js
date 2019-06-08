require("./base.js");

var testSequence = function(sequenceString, expected)
{
  var sequence = evaluator.evaluate(quote + sequenceString + quote);
  testSequenceMatches(sequence, expected);
}

testSequence("1 2 3",
[
 { "0/1" : ["1"] },
 { "1/3" : ["2"] },
 { "2/3" : ["3"] },
]);

testSequence("1 2 [3]",
[
  { "0/1" : ["1"] },
  { "1/3" : ["2"] },
  { "2/3" : ["3"] },
]);

testSequence("~ 2 3",
[
  { "1/3" : ["2"] },
  { "2/3" : ["3"] },
]);

testSequence("1 2 3, 4 5",
[
  { "0/1" : ["1", "4"] },
  { "1/3" : ["2"] },
  { "1/2" : ["5"] },
  { "2/3" : ["3"] },
]);

testSequence("1 2 3, 4 5 flood",
[
  { "0/1" : ["1", "4"] },
  { "1/3" : ["2", "5"] },
  { "2/3" : ["3", "flood"] },
]);

testSequence("1 2 3, 4 5 [6, C4]",
 [
   { "0/1" : ["1", "4"] },
   { "1/3" : ["2", "5"] },
   { "2/3" : ["3", "6", "C4"] },
 ]);
