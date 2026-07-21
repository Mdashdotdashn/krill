var fs = require("fs");

require("../js/input-evaluator.js");

function main()
{
  var evaluator = new Evaluator();

  var casesDoc = JSON.parse(fs.readFileSync("./tests/test-cases.json", "utf8"));
  var sources = Object.keys(casesDoc.cases);

  var astCases = {};
  sources.forEach(function(source) {
    astCases[source] = evaluator.evaluate(source);
  });

  var output = {
    generatedBy: "tests/update-ast-cases.js",
    cases: astCases
  };

  fs.writeFileSync("./tests/test-cases-ast.json", JSON.stringify(output, null, 2) + "\n");
  console.log("Wrote tests/test-cases-ast.json for " + sources.length + " cases.");
}

main();
