# Tidal Query Rebuild Plan

This document records the current migration plan in the repository so it can be committed and shared across machines.

## Goal

Rebuild rendering semantics toward Tidal-style arc queries by tearing down the current rendering-node runtime and reintroducing behavior case by case. The parser and AST pipeline remain in place, while runtime behavior is rebuilt by promoting accepted legacy cases into the supported corpus and implementing only the query-native node slices needed to make those cases green.

## Query Contract

The rewrite target is a node-level query API. Query-native nodes are the source of truth for runtime behavior; any player logic that remains exists only to consume query results for host scheduling.

### Request

A query takes a single absolute arc request:

- `start`: absolute start time as a rational fraction.
- `end`: absolute end time as a rational fraction.
- `arc`: the half-open interval `[start, end)`.

Rules:

1. Time is absolute. Queries are not relative to the current cycle or previous fetch.
2. Bounds are half-open: include events overlapping `start`, exclude events beginning exactly at `end`.
3. Requests may be shorter than one cycle, exactly one cycle, or span multiple cycles.
4. The same request against the same node state must return the same result in JS and C++.

### Response

A query returns an ordered list of event fragments. Each fragment carries:

- `whole`: the event's absolute logical arc before clipping.
- `part`: the intersection of `whole` with the requested arc.
- `value`: the event payload.

Rules:

1. Results are sorted by `part.start`, then by `whole.start` for deterministic ties.
2. `part` must always be contained inside the requested arc.
3. `whole` preserves the event's original extent even when `part` is clipped by the request.
4. Query results are data, not scheduling commands; the player derives next-event behavior from them.

### Boundary Semantics

Zero-width request:

- If `start == end`, return an empty result.
- Zero-width requests never synthesize edge events.

Single-cycle partial request:

- Return every event whose `whole` overlaps the requested arc.
- Clip each result into `part = whole intersect request`.

Cross-cycle request:

- If a request crosses a cycle boundary, split it at the boundary into per-cycle sub-queries.
- Query each sub-arc with the same semantics as any other request.
- Concatenate the sub-results in ascending time order.
- Preserve half-open boundaries during concatenation so boundary events are not duplicated.

Multi-cycle request:

- Apply the same split-and-concatenate rule across every crossed cycle boundary.
- A multi-cycle query is valid input, not a fallback case.

### Purity And State

1. `query` reads node state but does not advance it.
2. `tick` remains the explicit state-advance mechanism for nodes that need progression.
3. The player may call `tick` at cycle boundaries, but `query` itself must remain observational.
4. A rebuilt node is not complete until its behavior can be described in terms of `tick` plus `query`, not `next event at time` callbacks.

### Transition From Current Runtime

The rewrite does not adapt the current player and render-node runtime in place. The parser and AST pipeline remain, but the existing JS and C++ rendering-node implementations are not the target architecture and are not carried forward as a mixed execution lane.

During the rebuild:

1. The current runtime is treated as disposable implementation detail, not as a migration bridge.
2. New runtime behavior is introduced only through newly implemented query-native JS and C++ nodes.
3. Legacy runtime code may be consulted only as an external reference for expected behavior while selecting or understanding legacy cases; it is not the execution path being incrementally upgraded.
4. Supported behavior is reintroduced case by case by promoting accepted legacy cases into the supported corpus.
5. A runtime slice is considered rebuilt only when the minimal node behavior required for an accepted case has been reimplemented in both JS and C++, covered by focused tests, and made green in the supported corpus.
6. Final architecture is defined by the rebuilt query-native nodes and their tests, not by compatibility with the old player-driven sequencing model.

## Rules

1. Keep the parser and AST pipeline intact unless a case explicitly requires parser work.
2. Do not preserve the current render-node runtime as a mixed execution lane; rebuilt behavior must come from newly implemented query-native nodes.
3. Baseline scope is to migrate every legacy case into the rebuilt runtime; skipping a case is an exception that should be decided during development and reflected by removing that case from the legacy corpus source.
4. A runtime slice is considered rebuilt only when the minimal JS and C++ node behavior required for the selected legacy case is implemented, documented in the rebuilt [docs/render-operator-parity.md](render-operator-parity.md), locked by node-level unit tests on both sides, and green in the supported corpus.
5. Query coverage must include full-cycle, partial-cycle, zero-width, and multi-cycle requests, using the request/response contract above.
6. The current PR may run only the subset of tests that touches the rebuilt slice, but final acceptance still requires the full parity suite for all accepted supported cases.

## Corpus Policy

- [tests/test-cases.json](../tests/test-cases.json) is the supported corpus for the PR.
- `tests/test-cases-legacy.json` is created during the tear-down phase by copying the pre-rebuild contents of [tests/test-cases.json](../tests/test-cases.json).
- After that snapshot is taken, [tests/test-cases.json](../tests/test-cases.json) is intentionally reset to an empty `cases` object so the supported corpus starts from no rebuilt runtime cases while the test infrastructure remains runnable.
- The supported corpus should contain only accepted cases that are expected to stay green during the rewrite.
- The legacy corpus is the default rebuild backlog: cases are promoted into the supported corpus one by one as their minimal rebuilt runtime slice lands.
- If a case is intentionally dropped from scope during development, that decision is made by removing it from the legacy corpus source rather than leaving it as an ambiguous pending item.
- The legacy corpus is not the primary execution target for the PR; it is the source from which supported behavior is reintroduced.

## Migration Steps

1. Keep the parser and AST pipeline in place as the stable front end.
2. Remove the current render-node runtime and its node-specific execution model as the target implementation.
3. Snapshot the pre-rebuild supported corpus by copying [tests/test-cases.json](../tests/test-cases.json) to `tests/test-cases-legacy.json`.
4. Optionally snapshot `tests/test-cases-ast.json` in the same way if preserving the pre-rebuild AST fixture set is useful during teardown, even though parser and AST behavior are expected to stay in parity throughout the rewrite.
5. Clean [docs/render-operator-parity.md](render-operator-parity.md) to a query-rebuild baseline so legacy render-node mapping does not remain the active contract, and create the baseline template that rebuilt slices will fill in as they land.
6. Reset [tests/test-cases.json](../tests/test-cases.json) to an empty `cases` object so the supported corpus reflects that no rebuilt runtime cases remain immediately after tear-down while the test harness still runs.
7. Take one legacy case from `tests/test-cases-legacy.json`.
8. Identify the minimal missing JS and C++ query-native node behavior required to satisfy it.
9. Implement that runtime slice and add node-level unit tests that lock behavior on both sides.
10. Document the rebuilt slice contract in [docs/render-operator-parity.md](render-operator-parity.md).
11. Promote the case into the supported corpus once the rebuilt slice is green.
12. Verify arc boundary handling, deterministic query behavior, and operator parity for that slice before moving to the next legacy case.
13. If a legacy case is intentionally dropped from scope, remove it from `tests/test-cases-legacy.json` as part of that decision.
14. Continue until the supported corpus covers every remaining legacy case.

## Verification

- Always-on checks: keep parser AST parity and parser canonicalization green while runtime behavior is being rebuilt; parser behavior is expected to remain unchanged through teardown and rebuild.
- AST fixture continuity: if `tests/test-cases-ast.json` is snapshotted during tear-down, use it the same way as the legacy runtime corpus when diagnosing unexpected parser drift.
- Slice-level checks: run node-level unit tests in JS and C++ that lock the rebuilt query-node behavior for the selected case before promoting it.
- Contract checks: assert request shape, response shape, ordering, clipping, and non-mutation semantics.
- Boundary checks: exercise full-cycle, partial-cycle, zero-width, and multi-cycle requests.
- Corpus checks: the promoted supported case must be green in both JS and C++ before the next legacy case is taken up.
- Final gate: run the full parity suite, including every remaining legacy-derived supported case and any required embedded checks.

## Related Files

- [docs/render-operator-parity.md](render-operator-parity.md)
- [tests/test-run-cases.js](../tests/test-run-cases.js)
- [tests/test-parity-contract.js](../tests/test-parity-contract.js)
- [tests/test-parity-contract-all.sh](../tests/test-parity-contract-all.sh)
- [embedded/tests/tst_run_cases.cpp](../embedded/tests/tst_run_cases.cpp)
- [embedded/tests/tst_render_tree_mapping.cpp](../embedded/tests/tst_render_tree_mapping.cpp)
