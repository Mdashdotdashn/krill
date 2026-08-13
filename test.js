// Canonical JS test entrypoint lives under core/js/tests.
// Keep this root shim so legacy commands (`node test.js`) still run all tests.
require("./core/js/tests/test-runner.js");
