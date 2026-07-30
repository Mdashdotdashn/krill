var assert = require("assert");

console.log("Running tests...");
require("./test-evaluator.js")
require("./test-ast-cases.js")
require("./test-parser-canonicalization.js")
require("./test-offline-evaluator-seam.js")
console.log("Running test cases...");
require("./test-run-cases.js")
console.log("Running test player state machine...");
require("./test-player-state-machine.js")
require("./test-render-query-contract.js")
console.log("Running test render nodes...");
require("./test-render-nodes.js")
require("./test-harmony.js")
console.log("done...");
