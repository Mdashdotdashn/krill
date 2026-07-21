var fs = require("fs");
var assert = require("assert");

require("./base.js");

function runAstParityCases()
{
  var expectedDoc = JSON.parse(fs.readFileSync("./tests/test-cases-ast.json", "utf8"));
  var expectedCases = expectedDoc.cases;

  var parseEvaluator = new Evaluator();

  for (var source in expectedCases)
  {
    var expectedAst = expectedCases[source];
    var actualAst = parseEvaluator.evaluate(source);

    try
    {
      assert.deepStrictEqual(actualAst, expectedAst);
    }
    catch (err)
    {
      throw new Error("AST mismatch for source: " + source + "\n" + err.message);
    }
  }
}

runAstParityCases();
