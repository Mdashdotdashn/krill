var assert = require("assert");

console.log("Running tests...");
require("./tests/test-evaluator.js")
require("./tests/test-ast-cases.js")
require("./tests/test-parser-canonicalization.js")
require("./tests/test-offline-evaluator-seam.js")
console.log("Running test cases...");
require("./tests/test-run-cases.js")
console.log("Running test player state machine...");
require("./tests/test-player-state-machine.js")
require("./tests/test-render-query-contract.js")
console.log("Running test render nodes...");
require("./tests/test-render-nodes.js")
require("./tests/test-harmony.js")
console.log("done...");
