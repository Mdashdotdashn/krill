var parser = require("note-parser");

function parseNumber(value)
{
  var text = String(value).trim();
  if (/^[-+]?\d+(?:\.\d+)?$/.test(text))
  {
    return Number(text);
  }
  return null;
}

function midiToSharpName(midi)
{
  var names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
  var pitch = ((midi % 12) + 12) % 12;
  var octave = Math.floor(midi / 12) - 1;
  return names[pitch] + String(octave);
}

function transposeNote(value, semitones)
{
  var parsed = parser.parse(String(value));
  if (!parsed || parsed.midi === null || parsed.midi === undefined)
  {
    if (parsed && parsed.chroma !== undefined)
    {
      var fallbackMidi = parsed.chroma + 60;
      var transposedFallback = fallbackMidi + semitones;
      if (transposedFallback < 0 || transposedFallback > 127)
      {
        return null;
      }
      return midiToSharpName(transposedFallback);
    }
    return null;
  }

  var transposed = parsed.midi + semitones;
  if (transposed < 0 || transposed > 127)
  {
    return null;
  }
  return midiToSharpName(transposed);
}

function addValues(lhs, rhs)
{
  var lNum = parseNumber(lhs);
  var rNum = parseNumber(rhs);
  if (lNum !== null && rNum !== null)
  {
    return String(lNum + rNum);
  }

  if (lNum !== null && Number.isInteger(lNum))
  {
    var t1 = transposeNote(rhs, lNum);
    if (t1 !== null)
    {
      return t1;
    }
  }

  if (rNum !== null && Number.isInteger(rNum))
  {
    var t2 = transposeNote(lhs, rNum);
    if (t2 !== null)
    {
      return t2;
    }
  }

  return String(rhs);
}

AddRenderNode = function(lhs, rhs)
{
  this.lhs_ = lhs;
  this.rhs_ = rhs;
}

function overlaps(aStart, aEnd, bStart, bEnd)
{
  return aStart < bEnd && aEnd > bStart;
}

AddRenderNode.prototype.query = function(start, end)
{
  var lhsFragments = this.lhs_.query(start, end) || [];
  var rhsFragments = this.rhs_.query(start, end) || [];
  if (!lhsFragments.length || !rhsFragments.length)
  {
    return [];
  }

  var out = [];
  rhsFragments.forEach(function(rhsFragment)
  {
    lhsFragments.forEach(function(lhsFragment)
    {
      if (!overlaps(lhsFragment.partStart, lhsFragment.partEnd, rhsFragment.partStart, rhsFragment.partEnd))
      {
        return;
      }

      var overlapStart = lhsFragment.wholeStart > rhsFragment.wholeStart
        ? lhsFragment.wholeStart
        : rhsFragment.wholeStart;
      var overlapEnd = lhsFragment.wholeEnd < rhsFragment.wholeEnd
        ? lhsFragment.wholeEnd
        : rhsFragment.wholeEnd;

      out.push({
        wholeStart: overlapStart,
        wholeEnd: overlapEnd,
        partStart: rhsFragment.partStart,
        partEnd: rhsFragment.partEnd,
        value: addValues(lhsFragment.value, rhsFragment.value)
      });
    });
  });

  return out;
}
