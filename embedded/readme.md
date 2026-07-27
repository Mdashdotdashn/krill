# Embedded Krill

This directory contains the C++ parser and renderer used for the embedded-oriented side of Krill. It is not meant to replace the Javascript workflow at the root of the repository; the intent is to share the same musical model and test cases while keeping reusable lower-level components that are easier to adapt to embedded targets.

It is intentionally not a complete application runtime. There is no device-facing playback engine, no opinionated player layer, and no embedded app shell here. The goal is to provide building blocks that can be reused inside other systems without forcing a specific integration model.

The included rendering code stops at the point where cycles and events can be produced from parsed Krill structures. What happens after that, such as scheduling, transport, hardware I/O, or host-specific playback behavior, is expected to be owned by the system embedding these components.

To build the current code, start with:

```sh
./prepare_build.sh
```

## AST parity with JS

Parser parity between JS and C++ is enforced with a shared AST snapshot generated from the root test corpus.

- Source corpus: `tests/test-cases.json`
- Snapshot file: `tests/test-cases-ast.json`
- Snapshot generator (run from repo root):

```sh
npm run update-ast-cases
```

Validation runs on both sides:

- JS suite (`node test.js`) includes `tests/test-ast-cases.js`
- C++ suite includes `tests/tst_ast_cases.cpp`

If parity fails, the failing source expression is printed by the C++ test so semantic-action mismatches can be fixed quickly.

## Render/operator parity contract

Cross-runtime render-node and operator mapping is tracked in [../docs/render-operator-parity.md](../docs/render-operator-parity.md).

Use that file as the source of truth before introducing new operators or changing existing runtime semantics.

## Core principles

The embedded code is organized as a pipeline:

1. source text is parsed into a normalized JSON model
2. that JSON model is converted into a render tree
3. the render tree is advanced cycle by cycle to produce events

That scope is deliberate: this directory is concerned with parsing, modeling, and rendering musical structure, not with defining the final engine or player architecture of the host application.

The important design rule is that the JSON model is the contract between parser and renderer. Parser changes should produce a clearer or richer model. Renderer changes should consume that model without depending on parser internals.

## Parser principles

The parser entry point is [Parser](parser/Parser.hpp), which delegates to [KrillParser](parser/KrillParser.hpp).

The responsibilities are split on purpose:

- [parser/KrillGrammar.hpp](parser/KrillGrammar.hpp) defines the accepted syntax.
- [parser/KrillParser.cpp](parser/KrillParser.cpp) assigns meaning through semantic actions.
- [parser/XmlBuilder.cpp](parser/XmlBuilder.cpp) builds the normalized JSON structures passed to the rest of the system.

When adding a parser feature, keep these rules in mind:

- If the language accepts new syntax, start in the grammar.
- If the syntax maps to an existing concept, reuse the existing JSON shape when possible.
- If a new concept is needed, make it explicit in the JSON model instead of hiding it in parser-only behavior.
- Keep semantic actions focused on translation, not runtime behavior.

In practice, that means a new operator or notation should usually require three changes: extend the grammar, add or update the semantic action, and add parser tests.

## Renderer principles

The renderer starts from the JSON model and builds runtime objects through [RenderTreeBuilder](renderer/RenderTreeBuilder.hpp).

The core abstraction is [RenderNode](renderer/RenderNode.hpp): each node can be ticked and can render a [Cycle](cycle/Cycle.hpp). Composite patterns, operators, and leaf elements are all expressed as node types.

The main responsibilities are:

- [renderer/RenderTreeBuilder.cpp](renderer/RenderTreeBuilder.cpp) maps JSON node types to render nodes.
- [renderer/nodes/](renderer/nodes/) contains the actual runtime behavior for patterns and operators.
- [renderer/RenderTreePlayer.hpp](renderer/RenderTreePlayer.hpp) advances the tree over time and emits events from rendered cycles.

### Query fragment semantics

`RenderNode::query` uses a half-open window `[start, end)` and returns one or more `QueryFragment` values.

- `wholeStart` / `wholeEnd`: full interval where a value is active in the node timeline.
- `partStart` / `partEnd`: clipped intersection of that interval with the current query request.

This split lets downstream code keep both pieces of information:

- event identity and source duration from `whole*`
- exact window-local slice from `part*`

When adding renderer features, prefer these rules:

- Put musical behavior in a render node or in builder dispatch, not in the player.
- Keep playback generic; [RenderTreePlayer](renderer/RenderTreePlayer.hpp) should not need feature-specific branches.
- Treat `tick()` and `render()` as the stable runtime contract.
- Reuse existing composition patterns when possible: unary operators wrap a child, binary operators combine children, and pattern nodes manage alignment and timing.

If a feature changes how an already-parsed structure behaves over time, it probably belongs in a new node type or a new case in the render tree builder.

## Guidelines for new features

Use this rule of thumb when deciding where to work:

- New syntax or notation: start in [parser/KrillGrammar.hpp](parser/KrillGrammar.hpp) and [parser/KrillParser.cpp](parser/KrillParser.cpp).
- New normalized model shape: update [parser/XmlBuilder.cpp](parser/XmlBuilder.cpp) and the corresponding renderer dispatch.
- New runtime transformation or operator behavior: add or update a node in [renderer/nodes/](renderer/nodes/) and wire it in [renderer/RenderTreeBuilder.cpp](renderer/RenderTreeBuilder.cpp).
- New end-to-end musical behavior: cover it in both parser-focused and render/playback-focused tests.

Try to avoid shortcuts that blur layers. A parser feature should not require custom playback logic. A renderer feature should not require the parser to encode execution details that belong in node behavior.

## Where to look first

These files are the best entry points when changing the embedded code:

- [parser/KrillGrammar.hpp](parser/KrillGrammar.hpp) for syntax.
- [parser/KrillParser.cpp](parser/KrillParser.cpp) for semantic mapping.
- [renderer/RenderTreeBuilder.cpp](renderer/RenderTreeBuilder.cpp) for JSON-to-runtime dispatch.
- [renderer/RenderNode.hpp](renderer/RenderNode.hpp) for the renderer contract.
- [renderer/RenderTreePlayer.hpp](renderer/RenderTreePlayer.hpp) for cycle playback.
- [tests/tst_parser.cpp](tests/tst_parser.cpp) for parser behavior.
- [tests/tst_run_cases.cpp](tests/tst_run_cases.cpp) for end-to-end behavior using the shared JS cases.

If a feature spans multiple layers, the usual order is: grammar, semantic mapping, JSON model, render node, end-to-end test.
