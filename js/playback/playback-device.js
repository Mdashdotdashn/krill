var easymidi = require('easymidi');
require('../music/conversion.js');

var defaultLoopback = function()
{
  var devices =
  {
  	dar: "iac",
  	win: "loop",
  	lin: "through"
  }

  var platformKey = process.platform.substring(0, 3);
  return devices[platformKey];
}

var findMidiDevice = function(name)
{
  const deviceName = (name ? name : defaultLoopback()).toLowerCase();

  const devices = easymidi.getOutputs();
  var device = devices.reduce((c,d) => {
    if (!c && (d.toLowerCase().includes(deviceName)))
    {
      console.log("Using midi interface "+ d);
      return new easymidi.Output(d);
    }
    return c;
  }, null);

  if (device) return device;

  console.log("ERR: cannot find midi device " + deviceName);
  console.log("Use one of the following:");
  console.log(easymidi.getOutputs());
  process.exit();
}

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
     return c.concat(notes);
   },[]);
	processNotes(player.values_, 'noteon');
}

////////////////////////////////////////////////////////////////////////////////

GMDevice = function(midiDeviceName)
{
  this.midiDevice_ = findMidiDevice(midiDeviceName);
  this.values_ = undefined;
}

GMDevice.prototype.hush = function()
{
  tickPlayer(this,{values: []});
}

GMDevice.prototype.tick = function(events)
{
  tickPlayer(this, events);
}

////////////////////////////////////////////////////////////////////////////////
