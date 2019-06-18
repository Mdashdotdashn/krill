var easymidi = require('easymidi');
var parser = require('note-parser')

console.log(easymidi.getOutputs());

var convertToNoteNumber = function(x)
{
  // first let's see if it is a number
  var parsedInt = parseInt(x);
  var parsedNote = parser.parse(x);
  return isNaN(parsedInt) ? (parsedNote ? parsedNote.midi : undefined) : parsedInt + 32;
}

////////////////////////////////////////////////////////////////////////////////

GMDevice = function(midiDeviceName)
{
  this.midiDevice_ = new easymidi.Output(midiDeviceName);
  this.values_ = undefined;
}

GMDevice.prototype.tick = function(event)
{
	var device = this.midiDevice_;

	processNotes = function(v,m)
	{
		if (v) v.forEach(function(x) {
			device.send(m, {
			  note: x.note,
			  velocity: 127,
			  channel: x.channel
			});
		});
	}
	processNotes(this.values_, 'noteoff');

  this.values_ = event.values.reduce((c,x) => {
     var noteNumber = convertToNoteNumber(x);
     if (noteNumber) {
       c.push({note: noteNumber, channel: 0});
     }
     return c;
   },[]);
  console.log(this.values_);
	processNotes(this.values_, 'noteon');
}

VCVDevice = function(midiDeviceName)
{
  this.midiDevice_ = new easymidi.Output(midiDeviceName);
  this.values_ = undefined;
}

VCVDevice.prototype.tick = function(event)
{
	var device = this.midiDevice_;

	processNotes = function(v,m)
	{
		if (v) v.forEach(function(x) {
			device.send(m, {
			  note: x.note,
			  velocity: 127,
			  channel: x.channel
			});
		});
	}
	processNotes(this.values_, 'noteoff');

  this.values_ = event.values.reduce((c,x) => {
     var noteNumber = convertToNoteNumber(x);
     if (noteNumber) {
       c.push({note: noteNumber, channel: 0});
     }
     return c;
   }, []);
  console.log(this.values_);
	processNotes(this.values_, 'noteon');
}

////////////////////////////////////////////////////////////////////////////////
