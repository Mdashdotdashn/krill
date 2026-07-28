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

function valuesAtTime(player, expectedTime)
{
  var values = player.eventsAtTime(expectedTime);
  if (!values)
  {
    return [];
  }
  return values.map(function(v) { return String(v); });
}

function eventValues(values)
{
  if (!values || values.length === 0)
  {
    return [];
  }
  return values.map(function(v) { return String(v); });
}

function sortExpectedTimes(expected)
{
  return Object.keys(expected).sort(function(a, b) {
    return math.compare(math.fraction(a), math.fraction(b));
  });
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
    player.reset();

    var expectedTimes = sortExpectedTimes(expected);
    var currentTime = math.fraction(-1, 10000);

    for (var i = 0; i < expectedTimes.length; i++)
    {
      var expectedTime = expectedTimes[i];
      var nextTime = null;
      var values = null;
      var guard = 0;

      while (!values || values.length === 0)
      {
        nextTime = player.nextOnsetTimeFrom(currentTime);
        values = player.eventsAtTime(nextTime);
        currentTime = nextTime;
        guard += 1;

        if (guard > 4096)
        {
          throw new Error("Stuck while advancing player for case: " + source);
        }
      }

      assert.strictEqual(
        fracToString(nextTime),
        fracToString(expectedTime),
        "Unexpected event time for case: " + source
      );

      var actualValues = eventValues(values);
      assert.deepStrictEqual(actualValues, expected[expectedTime], "Case failed: " + source + " @ " + expectedTime);
    }

  }
}

function runNoUnexpectedBetweenChecks()
{
  var evaluator = new Evaluator();
  var builder = new RenderingTreeBuilder();

  var exhaustiveCases = [
    {
      source: "'a b c d'",
      onsets: ["0", "1/4", "1/2", "3/4", "1"]
    },
    {
      source: "'[bd sd](2,8)'",
      onsets: ["0", "1/4", "1/2", "3/4", "1"]
    },
    {
      source: "struct 't f f t' $ 'bd'",
      onsets: ["0", "1/4", "1/2", "3/4", "1"]
    }
  ];

  exhaustiveCases.forEach(function(testCase) {
    var tree = builder.rebuild(evaluator.evaluate(testCase.source));
    var player = new RenderingTreePlayer();
    player.setRenderingTree(tree);
    player.reset();

    for (var i = 0; i + 1 < testCase.onsets.length; i++)
    {
      var from = testCase.onsets[i];
      var expectedNext = testCase.onsets[i + 1];
      var actualNext = player.nextOnsetTimeFrom(from);
      assert.strictEqual(
        fracToString(actualNext),
        fracToString(expectedNext),
        "Unexpected onset between " + from + " and " + expectedNext + " for case: " + testCase.source
      );
    }
  });
}

runAllTestCases();
runNoUnexpectedBetweenChecks();
