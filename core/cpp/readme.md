# C++ Implementation

PEG-based parser, render tree builder, and operator implementations for Krill. Reference implementation used for parity validation against the JavaScript version.

## Structure

```
cpp/
├── src/
│   ├── parser/          # KrillParser (PEG), Context, XmlBuilder
│   ├── renderer/        # RenderTreeBuilder, RenderNode, factories
│   └── harmony/         # Scale, Roman, NoteMidi, Interval
├── tests/               # Catch2 unit tests
├── third_party/         # cpp-peglib, rapidjson, FractionClass, etc.
├── CMakeLists.txt
├── prepare_build.sh
└── README.md
```

## Building

### Prerequisites
- C++17 compiler (GCC, Clang, MSVC)
- CMake 2.8+

### Build Steps

```bash
cd core/cpp

# First time setup
./prepare_build.sh

# Build
cd build && cmake --build .

# Run all tests
./Tests.exe

# Run specific tests
./Tests.exe "*Parser*"
./Tests.exe "*AST parity*"
```

## Key Components

- **parser/KrillParser.hpp** — Main PEG parser using cpp-peglib
- **renderer/RenderTreeBuilder.hpp** — Converts AST → render tree
- **renderer/RenderNode.hpp** — Base class for operators
- **renderer/factories/** — Operator implementations
- **harmony/** — Music theory (Scale, Roman, NoteMidi, Interval)

## Test Fixtures

Tests reference shared test cases in `../../core/test-cases.json` and `../../core/test-cases-ast.json`.

- `tst_ast_cases.cpp` — AST parity validation
- `tst_run_cases.cpp` — Full evaluation
- `tst_render_node_*.cpp` — Operator tests
- `tst_harmony_*.cpp` — Music theory tests

## Parity with JavaScript

C++ AST output is validated against `test-cases-ast.json`. If a test fails:

1. The failing source expression is printed to help diagnose the issue
2. If the change is intentional (grammar update), regenerate snapshots:
   ```bash
   npm run update-ast-cases  # from root
   ```
3. If unintentional, fix the parser

## Design Philosophy

The C++ code is organized as a reusable pipeline, not a complete application:

1. **Parser** — text → normalized JSON AST
2. **Renderer** — AST → render tree
3. **Evaluation** — render tree → events per cycle

This layer stops before playback/scheduling/device I/O—those are owned by the embedding system. The JSON model is the contract between parser and renderer.

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

The core abstraction is [RenderNode](renderer/RenderNode.hpp): each node answers query windows and emits `QueryFragment` slices over time. Composite patterns, operators, and leaf elements are all expressed as node types.

The main responsibilities are:

- [renderer/RenderTreeBuilder.cpp](renderer/RenderTreeBuilder.cpp) maps JSON node types to render nodes.
- [renderer/nodes/](renderer/nodes/) contains the actual runtime behavior for patterns and operators.
- [renderer/RenderTreePlayer.hpp](renderer/RenderTreePlayer.hpp) advances the tree over time and emits events from query windows.

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
- Treat `query()` as the stable runtime contract.
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
