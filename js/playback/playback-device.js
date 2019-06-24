var easymidi = require('easymidi');
require('../music/conversion.js');

console.log(easymidi.getOutputs());

var tickPlayer = function(player, events)
{
  var device = player.midiDevice_;

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

	processNotes(player.values_, 'noteoff');

  player.values_ = events.values.reduce((c,x) => {
     const notes = convertToNotes(x);
     const mapped = notes.map((n) => { return {note: n, channel: 0}});
     console.log(mapped);
     return c.concat(mapped);
   },[]);
  console.log(player.values_);
	processNotes(player.values_, 'noteon');
}

////////////////////////////////////////////////////////////////////////////////

GMDevice = function(midiDeviceName)
{
  this.midiDevice_ = new easymidi.Output(midiDeviceName);
  this.values_ = undefined;
}

GMDevice.prototype.tick = function(events)
{
  tickPlayer(this, events);
}

VCVDevice = function(midiDeviceName)
{
  this.midiDevice_ = new easymidi.Output(midiDeviceName);
  this.values_ = undefined;
}

VCVDevice.prototype.tick = function(events)
{
  tickPlayer(this, events);
}

////////////////////////////////////////////////////////////////////////////////
