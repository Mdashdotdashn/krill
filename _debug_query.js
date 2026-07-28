var math = require('mathjs');
require('./js/input-evaluator.js');
require('./js/renderer/render-tree.js');
require('./js/playback/rendering-tree-player.js');

var e = new Evaluator();
var b = new RenderingTreeBuilder();

// Test what [a]*4 produces across the full cycle
var t = b.rebuild(e.evaluate("'[a]*4'"));
console.log("--- '[a]*4' fragments in various windows ---");
[[0,1],[0,0.25],[0.25,0.5],[0.5,0.75],[0.75,1],[-0.0001,1]].forEach(function(r) {
  var frags = t.query(math.fraction(r[0]), math.fraction(r[1]));
  var desc = frags.map(function(f){ return "ws="+f.wholeStart+" we="+f.wholeEnd; }).join(", ") || "(empty)";
  console.log("  query("+r[0]+","+r[1]+"): " + desc);
});

console.log("\n--- player test for '[a]*4' ---");
var player = new RenderingTreePlayer();
t = b.rebuild(e.evaluate("'[a]*4'"));
player.setTree(t);
player.reset();
console.log("cachedEvents_:", JSON.stringify(player.cachedEvents_.map(function(e){return {t:math.format(e.time), v:e.values}})));
console.log("nextOnsetTimeFrom(-1/10000) =", math.format(player.nextOnsetTimeFrom(math.fraction(-1,10000))));
