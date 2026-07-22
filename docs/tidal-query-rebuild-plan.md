# Tidal Query Rebuild Plan

This document records the current migration plan in the repository so it can be committed and shared across machines.

## Goal

Rebuild rendering semantics toward Tidal-style arc queries by tearing down and reimplementing the rendering nodes in dependency order. The current tree is back on the legacy player/node shape, so the rebuild starts from that baseline and replaces operator families directly rather than preserving a long-lived mixed lane.

## Rules

1. Replace operator families one by one in dependency order, rather than layering new behavior onto the old structure.
2. No operator family is considered rebuilt until its Tidal mapping oracle is documented in [docs/render-operator-parity.md](render-operator-parity.md) and its targeted JS and C++ tests are green.
3. The legacy implementation is kept only as a comparison oracle while each family is rebuilt.
4. Once a family is rebuilt, the player routes exclusively through the new implementation for that family.
5. Query coverage must include full-cycle, partial-cycle, zero-width, and multi-cycle requests.
6. The current PR may run only the subset of tests that touches the rebuilt families, but final acceptance still requires the full parity suite.

## Corpus Policy

- [tests/test-cases.json](../tests/test-cases.json) is the supported corpus for the PR.
- tests/test-cases-legacy.json is the legacy corpus source used to extract additional cases into the supported corpus.
- The supported corpus should reflect what is currently implemented and intended to stay green during the rewrite.
- The legacy corpus should be treated as a reference source, not as the primary execution target for the PR.

## Migration Steps

1. Rebuild operator family routing from the legacy baseline.
2. Keep the current JS and C++ render-node implementations only as the reference shape while each family is rewritten.
3. Replace each operator family in dependency order, with no permanent mixed-lane registry in the target design.
4. Once a family is rebuilt, route the player exclusively through the new implementation and retire the old path for that family.
5. Build the pull-query foundation only once the family rewrite is in place.
6. Verify arc boundary handling, deterministic query behavior, and operator parity before moving to the next family.

## Verification

- Baseline capture: run the parity contract against the current supported corpus.
- Family-level checks: run the smallest JS and C++ tests that cover the rebuilt family.
- Boundary checks: exercise full-cycle, partial-cycle, zero-width, and multi-cycle requests.
- Final gate: run the full parity suite, including the supported corpus and any required embedded checks.

## Related Files

- [docs/render-operator-parity.md](render-operator-parity.md)
- [tests/test-run-cases.js](../tests/test-run-cases.js)
- [tests/test-parity-contract.js](../tests/test-parity-contract.js)
- [tests/test-parity-contract-all.sh](../tests/test-parity-contract-all.sh)
- [embedded/tests/tst_run_cases.cpp](../embedded/tests/tst_run_cases.cpp)
- [embedded/tests/tst_render_tree_mapping.cpp](../embedded/tests/tst_render_tree_mapping.cpp)
