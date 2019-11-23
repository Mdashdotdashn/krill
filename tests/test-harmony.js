require("../js/music/harmony.js");
assert = require("assert");

const Detect = require("tonal-detect")
const Chord = require("tonal-chord")
const Note = require("tonal-note")

var scaleString = "a minor";
var builder = new ProgressionBuilder(scaleString)
builder.reset(scaleString);

var tokenArray = "1 7 vii IIb 6".split(" ");
var expected = "Am GM Gm BbM F".split(" ");

var chords = tokenArray.map(x => Detect.chord(builder.processToken(x)));
assert.equal(expected.length, chords.length);

var checkChordMatches = function(expected, actual)
{
  var buildChroma = function(a)
  {
    return Chord.notes(a).map(x => Note.chroma(x)).sort()
  }
  assert.deepEqual(buildChroma(expected),buildChroma(actual[0]));
}

expected.forEach((x,index) => checkChordMatches(x, chords[index]));
