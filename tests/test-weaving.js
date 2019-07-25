require("./base.js");

var testWeaving = function(leftSequenceString, rightSequenceString, mode, operation, expected)
{
  var sequenceL = evaluator.evaluatePattern(quote + leftSequenceString + quote);
  var sequenceR = evaluator.evaluatePattern(quote + rightSequenceString + quote);
  var result = weaveSequences(sequenceL, sequenceR, mode, operation);
  testSequenceMatches(result, expected);
}

// Use left structure and left data
testWeaving(
  "1 2",
  "a b c d",
  "left",
  (l,r) => l,
  [
   { "0/1" : ["1"] },
   { "1/2" : ["2"] },
  ]);

// Use left structure and right data
testWeaving(
  "1 2",
  "a b c d",
  "left",
  (l,r) => r,
  [
   { "0/1" : ["a"] },
   { "1/2" : ["c"] },
  ]);

// Use right structure and left data
testWeaving(
  "1 2",
  "a b c d",
  "right",
  (l,r) => l,
  [
   { "0/1" : ["1"] },
   { "1/4" : ["1"] },
   { "1/2" : ["2"] },
   { "3/4" : ["2"] },
  ]);

// Use both structure and concatenate data
testWeaving(
  "2 3",
  "4 5 6",
  "both",
  (l,r) => [ l[0] + r[0] ],
  [
   { "0/1" : ["24"] },
   { "1/3" : ["25"] },
   { "1/2" : ["35"] },
   { "2/3" : ["36"] },
  ]);
