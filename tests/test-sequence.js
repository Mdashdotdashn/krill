var assert = require("assert");
var math = require("mathjs");
var peg = require("pegjs");
var fs = require('fs');

require('../js/sequence.js');

////////////////////////////////////////////////////////////////////////////////

var quote ="\"";

var buf = fs.readFileSync( __dirname + '/../grammar.txt');
var parser = peg.generate(buf.toString());

function testSequence(string, expected)
{
  var sequenceArray = parser.parse(quote+string+quote);
  var sequence = new Sequence();
  sequence.renderArray(sequenceArray);
  var output = sequence.sequence_.reduce(function(a,x) { var o = new Object() ; o[x.time] = x.values; a.push(o); return a;}, []);
  assert.deepEqual(output,expected);
}

function testNextTime(string, time, expected)
{
  var sequenceArray = parser.parse(quote+string+quote);
  var sequence = new Sequence();
  sequence.renderArray(sequenceArray);
  // Test we've got either the expected value or 'undefined' (meaning the cycle finishing)
  var next = sequence.nextTimeFrom(math.fraction(time));
  var nextTime = next ? next.time : undefined;
  assert.deepEqual(nextTime, expected ? math.fraction(expected) : undefined);
}

////////////////////////////////////////////////////////////////////////////////

testNextTime("1 2 3, 4 5", math.number(0.5), "2/3");
testNextTime("1 2 3", math.number(0), "1/3");
testNextTime("1 2 3", math.fraction("1/3"), "2/3");
testNextTime("1 2 3", math.fraction("2/3"), undefined);

testSequence("1 2 3",
 [
   { "0/1" : ["1"] },
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
