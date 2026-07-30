# Parity & Cross-Language Tests

This folder contains validation tests that ensure the C++ and JavaScript implementations produce identical results.

## Files

- **test-parity-contract.js** — Runs JavaScript test suite (unit tests from core/js/tests/)
- **test-parity-contract-all.sh** — Runs full parity pipeline:
  1. JavaScript tests
  2. Compiles C++ code
  3. Runs C++ tests
  4. Validates both produced identical ASTs

## Running Parity Tests

### JavaScript Tests Only
```bash
npm run test-parity-contract-js
```

### Full Parity (JS + C++ + AST Validation)
```bash
npm run test-parity-contract-all
```

This will:
1. Run all JS tests from `core/js/tests/`
2. Build C++ from `core/cpp/` (cmake)
3. Run C++ tests (`core/cpp/build/Tests.exe`)
4. Compare AST snapshots to ensure both implementations agree

## What Parity Means

At the parity testing level, we validate two things:

1. **AST Parity** — Given the same input, both C++ and JavaScript parsers produce identical AST structures. This is checked by:
   - Both implementations parse shared test cases from `core/test-cases.json`
   - Both validate against golden AST snapshot `core/test-cases-ast.json`
   - If output differs, the test fails with the differing source expression

2. **Operator Parity** — Both implementations support the same set of render operators and produce identical render trees. Documented in [../docs/render-operator-parity.md](../docs/render-operator-parity.md).

## Workflow

When making changes that affect parsing or rendering:

1. **Make the change** in both C++ and JavaScript (or just JS if platform-specific)
2. **Run tests in your implementation** (e.g., `npm test` for JS)
3. **Run parity tests** — `npm run test-parity-contract-all`
4. **If parity fails:**
   - Check the error message for the failing source expression
   - Fix the implementation (usually parser semantics or operator behavior)
   - Re-run parity tests
5. **If parity change is intentional** (e.g., grammar update):
   - Regenerate AST snapshot: `npm run update-ast-cases` (from root)
   - Commit both code changes and updated snapshot

## Build Requirements

### For JavaScript Tests
- Node.js (any recent version)
- Dependencies: `npm install`

### For C++ Tests
- C++17 compiler
- CMake 2.8+
- Run `core/cpp/prepare_build.sh` for first-time setup

The shell script `test-parity-contract-all.sh` checks for the C++ build directory and fails with helpful instructions if it doesn't exist.
