const Tonal = require('tonal');
const Scale = require('tonal-scale');
const Chord = require('tonal-chord');
const Interval = require('tonal-interval');
const Distance = require('tonal-distance');
const RomanNumeral = require("tonal-roman-numeral");
const Detect = require("tonal-detect")

// returns the base chord for a given scale degree (0-6)

var degreeChord = function (scaleDef, degree)
{
  const note = scaleDef.notes[degree];

  const chordIntervals= [0,2,4].map(d => {
    const offset = d + degree;
    const octave = Interval.fromSemitones(12 * Math.floor(offset / scaleDef.intervals.length));
    const interval = scaleDef.intervals[offset % scaleDef.intervals.length];
    return Distance.add(interval, octave);
  }, this);

    // We need to recover the chord type from the interval list.
  // Since tonal list the result from (C->B) in the case there's several options
  // we first re-root the progression so the first interval is 1P
  const backToRoot = "-" + chordIntervals[0];
  const transposed = chordIntervals.map(Distance.add(backToRoot));
  const notes = transposed.map(Distance.transpose('C'));
  const detected = Detect.chord(notes);

  const props = Chord.tokenize(detected[0]);
  const chordType = props[1] =='64' ? "M" : props[1];
  const chordName = note + chordType;
  return chordName;
}

// Apply inversion to a group of (sorted) notes

var invertChord = function(source, amount)
{
  const sign = Math.sign(amount);
  const count = Math.abs(amount);
  const offset = Interval.fromSemitones(12 * sign);
  var notes = source;
  for (var i = 0; i< count; i++)
  {
    switch(sign)
    {
      case 1:
        var lowest = notes.shift();
        notes.push(Tonal.transpose(lowest, offset));
        break;

      case -1:
        var highest = notes.pop();
        notes.unshift(Tonal.transpose(highest, offset));
        break;
    }
  }
  return notes;
}

// Computes the center of gravity for a group of notes

var noteGravityCenter = function(notes)
{
  var sum = 0.0;
  notes.forEach(function(note)
    {
      sum += Tonal.Note.midi(note);
    });
  return sum / notes.length;
}

// Computes the distance between two chords

var computeDistance = function(noteList1, noteList2)
{
  return noteGravityCenter(noteList2) - noteGravityCenter(noteList1);
}

// Computes the ideal inversion in order to minimize some form of distance
var rectifyChord = function(noteListFrom, noteListTo)
{
  var d = computeDistance(noteListFrom, noteListTo);
  var sign = Math.sign(d);
  d = Math.abs(d);
  var mind = 1000;

  var result = noteListTo;
  while(mind > d)
  {
    mind =d;
    result = invertChord(result, -sign);
    d = Math.abs(computeDistance(noteListFrom, result));
  }
  result = invertChord(result, sign);
  return result;
}

//
// -----------------------------------------------------------------------------
//


// Processes a single token and returns the list of intervals making a triad
// Expected:
//    1,2,3,4,5,6,7 -> Returns the triad corresponding to the given scale degree
//    I,II,III,IV,V,VI,VII -> Returns the major triad based on the scale degree
//      (can also be used as 1M,2M,3M,...)
//    i,ii,iii,iv,v,vi,vii -> Returns the minor tried based on the scale degree
//      (can also be used as 1m,2m,3m,...)
//    Any chords -> chord triad in base position
//    Neo-remanian
//    Relative (aka) +3m, +5M, -1m -> build a chord from the current by going up/down a certain amount of semi-tones and using a chord of quality(m/M)
//      See https://www.youtube.com/watch?v=YSKAt3pmYBs & the Ben Levin Radiohead chord chord Generator

var parseChordFromToken = function(x, scaleDefinition)
{
  // Tries to identify though various methods and build a chord from it
  // For example, 5 in C Major should return CMaj

  // is the token a chord?
  const token = Chord.tokenize(x);
  if ((x[0] != "") && (Chord.exists(x[1])))
  {
    return x;
  }

  // is the token a number ?
  var parsed = parseInt(x);
  if (!isNaN(parsed))
  {
    return degreeChord(scaleDefinition,parsed - 1);
  }

  // is the torken a numeral ?
  const props = RomanNumeral.props(x);
  if (props.name)
  {
    const note = scaleDefinition.notes[props.decimal - 1 ];
    const quality = props.major ? "M" : "m";
    return note + props.type + quality ;
  }
}

ProgressionBuilder = function()
{
}

ProgressionBuilder.prototype.reset = function(scaleString)
{
  const split = Scale.tokenize(scaleString);

  this.scaleDefinition_ =
  {
    root: split[0],
    name: split[1],
    intervals: Scale.intervals(split[1]),
    notes: Scale.notes(split[0], split[1])
  };

  this.currentChord_ = new Array();
}

ProgressionBuilder.prototype.processToken = function(x)
{
  var offset = 0;

  // parse chord from the given token
  const chord = parseChordFromToken(x, this.scaleDefinition_);

  // extract and sort the chord's notes
  var notes = Chord.notes(chord).map(x => x+"3");
  notes.sort(function(a,b)
  {
    return Tonal.Note.midi(a) - Tonal.Note.midi(b);
  });

  if (notes.length > 0)
  {
    // If we have a previous chord, apply automatic voicing
    if (this.currentChord_.length > 0)
    {
      notes = rectifyChord(this.currentChord_, notes);
    }
    else
    {
      notes = invertChord(notes, offset);
    }
    this.currentChord_ = notes;
  }
  return notes
}
