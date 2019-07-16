var parser = require('note-parser');
var chord = require('tonal-chord');

var drumMap = new Array();
drumMap["bd"] = 36;
drumMap["sd"] = 38;
drumMap["hh"] = 42;
drumMap["oh"] = 46;
drumMap["clave"] = 75;

convertToNotes = function(x)
{
  var notes = [];
  // first let's see if it is a number
  const parsedInt = parseInt(x);
  if (!isNaN(parsedInt)) {
    notes.push(parsedInt + 36);
  }
  else {
    // Then let's ee if it is a note name
    const parsedNote = parser.parse(x);
    if (parsedNote)
    {
      if (parsedNote.midi)
      {
        notes.push(parsedNote.midi);
      }
      else
      {
        notes.push(parsedNote.chroma + 62);
      }
    }
    else
    {
      // Is it a chord
      const parsedChord = chord.notes(x);
      if (parsedChord.length > 0)
      {
        notes = parsedChord.map((x) => convertToNotes(x));
      }
      else
      {
        // is it a drum name
        if(drumMap[x]) notes.push(drumMap[x]);
      }
    }
  }
  return notes;
}
