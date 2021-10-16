require("../patterns/weaving.js");

var parser = require('note-parser');
var note = require('tonal-note');

makeAddOperator = function(source, pattern)
{
  var applyFn = function(args)
  {
    var operator = function(l,r)
    {
      // Then let's ee if it is a note name
      const parsedNote = parser.parse(l);
      if (parsedNote && parsedNote.midi)
      {
        return note.fromMidi(parsedNote.midi + parseFloat(r), true);
      }
      return parseFloat(l) + parseFloat(r);
    }
    return weavePatterns(args[0], args[1], "both", operator);
  }

  return new Operator(applyFn, [source, pattern]);
}
