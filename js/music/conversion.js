var parser = require('note-parser');
var chord = require('tonal-chord');

convertToNotes = function(x)
{
  var notes = [];
  // first let's see if it is a number
  const parsedInt = parseInt(x);
  if (!isNaN(parsedInt)) {
    notes.push(parsedInt + 36);
  }
  else {
    const parsedNote = parser.parse(x);
    if (parsedNote)
    {
      console.log(parsedNote);
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
      const parsedChord = chord.notes(x);
      console.log(parsedChord);
      if (parsedChord.length > 0)
      {
        notes = parsedChord.map((x) => convertToNotes(x));
      }
    }
  }
  return notes;
}
