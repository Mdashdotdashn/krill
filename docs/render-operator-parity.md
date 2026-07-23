# Render Operator Parity (JS/C++)

This document is the parity baseline for the query-rebuild phase.

The legacy render-node mapping matrix was intentionally cleared during teardown.
From this point onward, entries are added back only when a rebuilt query-native
slice is implemented and locked by tests on both JS and C++.

## Rebuild Scope

- Parser and AST parity remain continuous and are validated independently.
- Runtime parity was rebuilt case by case from the legacy corpus during migration.
- `tests/test-cases.json` now contains the fully rebuilt accepted corpus.

## Completion Status

- Legacy-to-supported promotion is complete.
- The temporary legacy corpus file used during migration has been removed.
- Full parity gates are green at completion.

## Query Runtime Contract

Each rebuilt slice must satisfy the query contract defined in
[docs/tidal-query-rebuild-plan.md](tidal-query-rebuild-plan.md):

- absolute request arc `[start, end)`
- ordered fragment response (`whole`, `part`, `value`)
- deterministic ordering
- query purity (`query` does not mutate state)
- explicit state progression through `tick`

## Rebuilt Slice Checklist

A slice is considered rebuilt only when all of the following are true:

1. The minimal query-native behavior for the selected legacy case is implemented in JS.
2. The minimal query-native behavior for the selected legacy case is implemented in C++.
3. Node-level unit tests lock behavior in JS.
4. Node-level unit tests lock behavior in C++.
5. The selected case is promoted into `tests/test-cases.json` and is green in both runtimes.
6. The slice contract is documented in this file.

## Slice Ledger

The per-slice ledger requirement applied during active migration.
The migration is now complete; see git history for the full per-slice trail.

| Legacy case | Query slice summary | JS tests | C++ tests | Supported corpus status | Notes |
|---|---|---|---|---|---|

## Test Gates During Rebuild

Always-on parser and AST gates:

- JS: `tests/test-ast-cases.js`
- JS: `tests/test-parser-canonicalization.js`
- C++: `embedded/tests/tst_ast_cases.cpp`
- C++: `embedded/tests/tst_parser.cpp` (canonicalization section)

Slice-level runtime gates (per promoted case):

- Node-level JS tests for the rebuilt slice
- Node-level C++ tests for the rebuilt slice
- Shared supported-corpus case check on JS and C++

## Change Policy

Historical policy used during migration (kept for reference):

1. Add or update node-level tests first.
2. Implement the minimal JS and C++ runtime slice.
3. Promote the case into `tests/test-cases.json`.
4. Add a slice entry to the ledger above.
5. Keep parser/AST gates green throughout.
