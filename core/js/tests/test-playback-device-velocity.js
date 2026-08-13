var assert = require("assert");

require("../playback/playback-device.js");

function makeFragment(value, controls)
{
  return {
    value: value,
    controls: controls
  };
}

(function testGMDeviceUsesDefaultVelocityWhenUnspecified()
{
  var messages = [];
  var device = Object.create(GMDevice.prototype);
  device.midiDevice_ = {
    send: function(message, options)
    {
      messages.push({ message: message, options: Object.assign({}, options) });
    }
  };

  device.tick({ time: "0", values: ["bd"] });

  assert.deepStrictEqual(messages, [{
    message: "noteon",
    options: { note: 36, velocity: 127, channel: 9 }
  }]);
})();

(function testGMDeviceResolvesFragmentVelocity()
{
  var messages = [];
  var device = Object.create(GMDevice.prototype);
  device.midiDevice_ = {
    send: function(message, options)
    {
      messages.push({ message: message, options: Object.assign({}, options) });
    }
  };

  device.tick({
    time: "0",
    fragments: [makeFragment("bd", { velocity: 0.8 })],
    values: ["bd"]
  });

  assert.deepStrictEqual(messages[0], {
    message: "noteon",
    options: { note: 36, velocity: 102, channel: 9 }
  });

  device.tick({ time: "1/4", values: [] });

  assert.deepStrictEqual(messages[1], {
    message: "noteoff",
    options: { note: 36, velocity: 102, channel: 9 }
  });
})();

(function testMidiFileRendererExportsNormalizedVelocity()
{
  var exported = [];
  var renderer = new MidiFileRenderer();
  renderer.setTempo(120);
  renderer.midiExporter_ = {
    header: {
      tempos: [],
      update: function() {}
    },
    addTrack: function()
    {
      return {
        addNote: function(note)
        {
          exported.push(Object.assign({}, note));
        }
      };
    },
    toArray: function()
    {
      return [];
    }
  };
  renderer.trackArray_ = [];

  renderer.tick("0", {
    time: "0",
    fragments: [makeFragment("bd", { velocity: 100, velocityFactor: 80 })],
    values: ["bd"]
  });
  renderer.tick("1/4", { time: "1/4", values: [] });

  assert.strictEqual(exported.length, 1);
  assert.strictEqual(exported[0].midi, 36);
  assert(Math.abs(exported[0].velocity - (63 / 127)) < 1e-9);
})();