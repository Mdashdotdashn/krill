var easymidi = require('easymidi');

var findMidiInputDevice = function(name)
{
  if (name)
  {
    var lowCaseName = name.toLowerCase();
    const devices = easymidi.getInputs();
    var device = devices.reduce((c,d) => {
      if (!c && (d.toLowerCase().includes(lowCaseName)))
      {
        console.log("Syncing from "+ d);
        return new easymidi.Input(d);
      }
      return c;
    }, null);

    if (device) return device;

    console.log("ERR: cannot find midi device " + name);
    console.log("Use one of the following:");
    console.log(easymidi.getInputs());
    process.exit();
  }
}


SyncDevice = function(midiDeviceName)
{
  device = findMidiInputDevice(midiDeviceName);
  if (device)
  {
    this.midiDevice_ = device;
  }
}

SyncDevice.prototype.enabled = function()
{
  return this.midiDevice_ != undefined;
}

SyncDevice.prototype.connect = function(target)
{
  if (this.midiDevice_)
  {
    device.on('clock', function(msg) {
      target.onSyncClock();
    });
    device.on('start', function(msg) {
      target.onSyncStart();
    });
    device.on('stop', function(msg) {
      target.onSyncStop();
    });
  }
}
