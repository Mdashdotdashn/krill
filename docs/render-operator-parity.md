# Render Operator Parity (JS/C++)

This document defines the runtime parity contract between the JS operator tree and the embedded C++ render-node tree.

Scope:
- Runtime construction and behavior from normalized AST nodes.
- Operator naming and argument-shape mapping.
- Intentional differences that must remain explicit.

Non-scope:
- Full language grammar details.
- Host playback and scheduling APIs.

## Builder mapping

- JS builder entry: [js/rendering-tree.js](../js/rendering-tree.js)
- C++ builder entry: [embedded/renderer/RenderTreeBuilder.cpp](../embedded/renderer/RenderTreeBuilder.cpp)

Both runtimes follow the same pattern:
1. Pattern and element nodes are built directly.
2. Operator nodes wrap a source child and consume arguments.
3. Object-valued operator arguments are normalized to a repeating one-cycle view for sampling-sensitive operators.

## Operator matrix

| AST type | JS runtime mapping | C++ runtime mapping | Parity status | Notes |
|---|---|---|---|---|
| `add` | `makeAddOperator` | `makeAddRenderNode` | Aligned | Binary weave/add behavior on sampled values. |
| `struct` | `makeStructOperator` | `makeStructRenderNode` | Aligned | Right pattern gates left content. |
| `shift` | `makeShiftOperator` | `makeShiftRenderNode` | Aligned | Supports static offset and expression argument node. |
| `stretch` | `makeStrechOperator` | `makeStretchRenderNode` | Aligned (naming differs) | JS symbol keeps legacy spelling `Strech`. |
| `trunc` | `makeTruncOperator` | `makeTruncRenderNode` | Aligned | Length truncation of rendered cycle. |
| `scale` | `makeScaleOperator` | `makeScaleRenderNode` | Aligned | Scale-name argument expected as string. |
| `bjorklund` | `makeBjorklundOperator` | `makeBjorklundRenderNode` | Aligned | C++ builds per-group subtree clones to avoid shared tick state. |
| `fixed-step` | `makeFixedStepOperator` | `makeFixedStepRenderNode` | Aligned | Step-division-driven stretch variant. |
| `pattern` (`h`) | `makePatternRenderingOperator` | `makeWeightedPatternRenderNode` | Aligned | Horizontal pattern composition. |
| `pattern` (`v`) | `makeStackRenderingOperator` | `makeStackRenderNode` | Aligned | Parallel stack composition. |
| `pattern` (`t`) | `makeTimelineOperator` | `makeTimelineRenderNode` | Aligned | Timeline/round-robin composition. |

## Timing transform canonicalization

At parser/AST level, both sides normalize several user-facing forms into runtime operator types:

- `slow n` -> `stretch [n]`
- `fast n` -> `stretch [1/n]`
- slice modifier `/n` -> `stretch [n]`
- slice modifier `*n` -> `stretch [1/n]`
- slice modifier `%n` -> `fixed-step [n]`

This contract means runtime trees do not require dedicated `fast` or `slow` node types.

## Argument normalization contract

For operators that sample another expression as an argument (`add`, `struct`, dynamic `shift`), object arguments are wrapped in a one-cycle normalization node:

- JS: `makeCycleNormalizeOperator(...)`
- C++: `NormalizeCycleRenderNode(...)`

Goal:
- Stable cycle-boundary behavior when argument expressions are shorter or longer than one cycle.

## Lifecycle invariants

The following runtime lifecycle behavior is part of the parity contract:

1. One-cycle slicing invariant.
- Normalization wrappers expose exactly one cycle per `render()` after sufficient `tick()` accumulation.

2. Boundary continuity invariant.
- Repeated `tick()` + `render()` calls on normalized argument nodes must produce stable cross-cycle progression for expression arguments.

3. Scalar argument invariant.
- Scalar shift/add/struct arguments are not wrapped in cycle-normalize wrappers; only AST object arguments are normalized.

4. Canonical transform invariant.
- `slow`/`fast` and slice modifiers must remain canonicalized to `stretch`/`fixed-step` at parser output, not reintroduced as separate runtime node kinds.

## Test gates

Changes touching parser or render-tree behavior should keep all of the following green:

1. Parser AST parity gate.
- JS: [tests/test-ast-cases.js](../tests/test-ast-cases.js)
- C++: [embedded/tests/tst_ast_cases.cpp](../embedded/tests/tst_ast_cases.cpp)

2. Shared runtime corpus gate.
- JS: [tests/test-run-cases.js](../tests/test-run-cases.js)
- C++: [embedded/tests/tst_run_cases.cpp](../embedded/tests/tst_run_cases.cpp)

3. Timing canonicalization gate.
- JS: [tests/test-parser-canonicalization.js](../tests/test-parser-canonicalization.js)
- C++: [embedded/tests/tst_parser.cpp](../embedded/tests/tst_parser.cpp) (`timing transform canonicalization` section)

4. Lifecycle wrapper gate.
- JS: [tests/test-operator.js](../tests/test-operator.js) (`testCycleNormalizeOperatorPrefetch`)
- C++: [embedded/tests/tst_renderNode.cpp](../embedded/tests/tst_renderNode.cpp) (`NormalizeCycleRenderNode slices one cycle per render`)

5. Render-structure mapping gate.
- JS: [tests/test-render-structure.js](../tests/test-render-structure.js)
- C++: [embedded/tests/tst_render_tree_mapping.cpp](../embedded/tests/tst_render_tree_mapping.cpp)

## Explicitly unsupported in runtime contract

- `target` operator dispatch is not part of the current JS/C++ runtime parity contract.
- New operators must be added to both builders and this matrix in the same change set.

## Change policy

When introducing or changing operator behavior:
1. Update this matrix first.
2. Update JS and C++ builders/nodes in the same branch.
3. Add or update tests on both sides.
4. Keep AST parity tests green.
