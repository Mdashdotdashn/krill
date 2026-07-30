var fs = require("fs");
var path = require("path");

require("../input-evaluator.js");

function main()
{
  var evaluator = new Evaluator();

  var casesDoc = JSON.parse(fs.readFileSync(path.join(__dirname, '../../test-cases.json'), "utf8"));
  var sources = Object.keys(casesDoc.cases);

  var astCases = {};
  sources.forEach(function(source) {
    astCases[source] = evaluator.evaluate(source);
  });

  var output = {
    generatedBy: "core/js/tests/update-ast-cases.js",
    cases: astCases
  };

  fs.writeFileSync(path.join(__dirname, '../../test-cases-ast.json'), JSON.stringify(output, null, 2) + "\n");
  console.log("Wrote core/test-cases-ast.json for " + sources.length + " cases.");
}

main();
