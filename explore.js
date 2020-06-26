var assert = require("assert");

require("./js/playback/rendering-tree-player.js");
require('./tests/base.js');

const renderingTree = evaluator.evaluateRenderingTree("'bd sd [hh hh] rs'");
Dump(renderingTree);
