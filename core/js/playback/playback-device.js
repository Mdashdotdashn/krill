var easymidi = require('easymidi');
const { Midi } = require('@tonejs/midi')
const fs = require('fs');

require('../music/conversion.js');

var DEFAULT_VELOCITY = 127;

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

function normalizeVelocityValue(value)
{
  var numeric = Number(value);
  if (!isFinite(numeric))
  {
    return DEFAULT_VELOCITY;
  }

  if (numeric >= 0 && numeric <= 1)
  {
    return Math.max(0, Math.min(DEFAULT_VELOCITY, Math.round(DEFAULT_VELOCITY * numeric)));
  }

  return Math.max(0, Math.min(DEFAULT_VELOCITY, Math.round(Math.abs(numeric))));
}

function combineVelocityValues(left, right)
{
  return Math.max(0, Math.min(DEFAULT_VELOCITY, Math.round(
    (normalizeVelocityValue(left) * normalizeVelocityValue(right)) / DEFAULT_VELOCITY
  )));
}

function resolveVelocity(fragment)
{
  var controls = fragment && fragment.controls && typeof fragment.controls === "object"
    ? fragment.controls
    : null;
  var velocity = controls && controls.velocity !== undefined ? controls.velocity : undefined;
  var velocityFactor = controls && controls.velocityFactor !== undefined ? controls.velocityFactor : undefined;

  var baseVelocity = velocity !== undefined ? normalizeVelocityValue(velocity) : DEFAULT_VELOCITY;
  var factorVelocity = velocityFactor !== undefined ? normalizeVelocityValue(velocityFactor) : DEFAULT_VELOCITY;

  return combineVelocityValues(baseVelocity, factorVelocity);
}

function noteObjectsFromValue(value, fragment)
{
  var velocity = resolveVelocity(fragment);
  return convertToNotes(value).map(function(note) {
    return Object.assign({}, note, { velocity: velocity });
  });
}

var tickPlayer = function(player, events)
{
  var device = player.midiDevice_;

	processNotes = function(v,m)
	{
		if (v) v.forEach(function(x) {
			device.send(m, {
			  note: x.note,
			  velocity: x.velocity,
        channel: x.channel
      });
		});
	}

	processNotes(player.values_, 'noteoff');

	if (events && Array.isArray(events.fragments) && events.fragments.length > 0)
	{
	  player.values_ = events.fragments.reduce((c, fragment) => {
	     return c.concat(noteObjectsFromValue(fragment.value, fragment));
	   }, []);
	}
	else
	{
	  player.values_ = (events && Array.isArray(events.values) ? events.values : []).reduce((c, value) => {
	     return c.concat(noteObjectsFromValue(value, null));
	   }, []);
	}
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

MidiFileRenderer = function()
{
  this.midiDevice_ = this;
  this.midiExporter_ = new Midi();
  this.trackArray_ = new Array();
  this.pendingNotes_ = new Array();
  this.tempo_ = 0;
}

MidiFileRenderer.prototype.tick = function(time, events)
{
  this.currentTime_ = time;
  tickPlayer(this, events);
}

MidiFileRenderer.prototype.send = function(message, options)
{
  var tempo = this.tempo_;
  var toDuration = (cycleTime) => { return cycleTime * 60 * 4 / tempo;}
  switch(message)
  {
    case 'noteon':
      // remove possible duplicates
      this.pendingNotes_ = this.pendingNotes_.filter((value) => {
        return (value.note != options.note)
      });
      options.time = this.currentTime_;
      this.pendingNotes_.push(options);
      break;
    case 'noteoff':
      this.pendingNotes_ = this.pendingNotes_.filter((value) => {
        if (value.note == options.note)
        {
          var noteData = {
              midi: value.note,
              time: toDuration(value.time),
              duration: toDuration(this.currentTime_ - value.time),
              velocity: normalizeVelocityValue(value.velocity) / DEFAULT_VELOCITY
          }
          if (!this.trackArray_[value.channel])
          {
            this.trackArray_[value.channel] = this.midiExporter_.addTrack();
          }
          this.trackArray_[value.channel].addNote(noteData);
          return false;
        }
        return true;
      });
      break;
  }
}

MidiFileRenderer.prototype.write = function(filename)
{
  fs.writeFileSync(filename,new Buffer(this.midiExporter_.toArray()))
}

MidiFileRenderer.prototype.setTempo = function(tempo)
{
  this.tempo_ = tempo;
  this.midiExporter_.header.tempos.push({
    bpm: tempo,
    ticks: 0,
  });
  this.midiExporter_.header.update();
}
