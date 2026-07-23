var fs = require("fs");
var assert = require("assert");
var math = require("mathjs");

require("../js/input-evaluator.js");
require("../js/renderer/render-tree.js");
require("../js/playback/rendering-tree-player.js");

function fracToString(v)
{
  return math.format(math.fraction(v));
}

function fragmentWholeStart(fragment)
{
  if (fragment.wholeStart !== undefined) return fracToString(fragment.wholeStart);
  if (fragment.whole && fragment.whole.start !== undefined) return fracToString(fragment.whole.start);
  if (fragment.start !== undefined) return fracToString(fragment.start);
  return null;
}

function fragmentValue(fragment)
{
  if (fragment.value !== undefined) return fragment.value;
  if (fragment.values !== undefined) return fragment.values;
  return undefined;
}

function valuesAtTime(player, expectedTime)
{
  var fragments = player.queryPointWindow(expectedTime);
  var result = [];
  fragments.forEach(function(fragment) {
    if (fragmentWholeStart(fragment) === expectedTime)
    {
      var value = fragmentValue(fragment);
      if (Array.isArray(value))
      {
        value.forEach(function(v) { result.push(String(v)); });
      }
      else if (value !== undefined)
      {
        result.push(String(value));
      }
    }
  });
  return result;
}

function runAllTestCases()
{
  var evaluator = new Evaluator();
  var builder = new RenderingTreeBuilder();
  var contents = fs.readFileSync("./tests/test-cases.json", "utf8");
  var testCases = JSON.parse(contents).cases || {};

  for (var source in testCases)
  {
    var expected = testCases[source];
    var model = evaluator.evaluate(source);
    var renderingTree = builder.rebuild(model);

    var player = new RenderingTreePlayer();
    player.setRenderingTree(renderingTree);

    for (var expectedTime in expected)
    {
      var actual = valuesAtTime(player, expectedTime);
      assert.deepStrictEqual(actual, expected[expectedTime], "Case failed: " + source + " @ " + expectedTime);
    }
  }
}

runAllTestCases();
