'use strict';
const program = require('commander');
require("./js/playback/rendering-tree-player.js");
require('./js/playback/playback-device.js');
require('./tests/base.js');
var math = require('mathjs');

program
  .version('0.0.1')
  .option('-t, --tempo <value>', 'sets the midi tempo', 120)
  .option('-d, --duration <value>', 'duration', 4)
  .option('-o, --output <value>', 'output file', "output.mid");

program.parse(process.argv);
const sequence = program.args[0];
if (!sequence)
{
  throw("error: no sequence provided");
}

// Make a rendering tree from the input
var renderingTree = evaluator.evaluateRenderingTree(sequence);
var renderingPlayer = new RenderingTreePlayer();
renderingPlayer.reset();
renderingPlayer.setRenderingTree(renderingTree);

var midiRenderer = new MidiFileRenderer();
midiRenderer.setTempo(program.opts().tempo);

const cycleCount = math.fraction(program.opts().duration);

var currentCycleTime = math.fraction("0");
while (currentCycleTime < cycleCount)
{
  var event = renderingPlayer.eventForTime(currentCycleTime);
  if (event)
  {
    midiRenderer.tick(currentCycleTime, event);
  }
  currentCycleTime = renderingPlayer.advance(currentCycleTime);
}

midiRenderer.write(program.opts().output);
console.log(program.opts().output+" written");
