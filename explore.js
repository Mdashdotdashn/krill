'use strict';
const program = require('commander');
var assert = require("assert");
var math = require('mathjs');

require("./js/playback/rendering-tree-player.js");
require('./tests/base.js');

program.parse(process.argv);
const sequence = program.args[0];
if (!sequence)
{
  throw("error: no sequence provided");
}

const renderingTree = evaluator.evaluateRenderingTree(sequence);
Dump(renderingTree);
