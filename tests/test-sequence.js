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
  var sequence = new Sequence(sequenceArray);
  var output = sequence.sequence_.reduce(function(a,x) { var o = new Object() ; o[x.time] = x.values; a.push(o); return a;}, []);
  assert.deepEqual(output,expected);
}

function testNextTime(string, time, expected)
{
  var sequenceArray = parser.parse(quote+string+quote);
  var sequence = new Sequence(sequenceArray);
  assert.deepEqual(sequence.nextTimeFrom(time), math.fraction(expected));
}

////////////////////////////////////////////////////////////////////////////////

testNextTime("1 2 3, 4 5", math.number(0.5), "2/3");
testNextTime("1 2 3", math.number(0), "1/3");
testNextTime("1 2 3", math.fraction("1/3"), "2/3");
testNextTime("1 2 3", math.fraction("2/3"), "1");
testNextTime("1 2 3", math.fraction("4/3"), "2/3");

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
