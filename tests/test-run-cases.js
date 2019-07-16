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
    var player = new SequencePlayer();
    player.setRenderingTree(renderingTree);
    var currentTime = "-0.0001";
    for (var expectedTime in expected)
    {
      var nextTime = player.advance(currentTime);
      assert.equal(fracToString(nextTime), expectedTime);
      const event = player.eventForTime(nextTime);
      assert.deepEqual(expected[expectedTime], event.values);
      currentTime = expectedTime;
    }
  } catch (err)
  {
    throw "Error trying to execute test case: " + test + "\n" + err;
  }
  }
}

runAllTestCases();
