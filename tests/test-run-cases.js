 var fs = require("fs");
 var _ = require("lodash");
 require('./base.js');

function runAllTestCases()
{
  var contents = fs.readFileSync("./tests/test-cases.json");
  var testCases = JSON.parse(contents).cases;
  for (var test in testCases)
  {
    try {
    var expected = testCases[test];
    console.log("> "+ test);
    const renderingTree = evaluator.evaluateRenderingTree(test);
    var player = new RenderingTreePlayer();
    player.setRenderingTree(renderingTree);
    var currentTime = "-0.0001";
    for (var expectedTime in expected)
    {
      // Look for the next event
      var event = undefined;
      while (!event)
      {
        var nextTime = player.advance(currentTime);
        event = player.eventForTime(nextTime);
        currentTime = fracToString(nextTime);
      }
      assert.equal(fracToString(nextTime), expectedTime);
      assert.deepEqual(expected[expectedTime], event.values);
    }
  } catch (err)
  {
    throw err; //"Error trying to execute test case: " + test + "\n" + err;
  }
  }
}

runAllTestCases();
