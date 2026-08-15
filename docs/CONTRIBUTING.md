# Contributing

Krill has parallel JavaScript and C++ implementations. Unless a change is explicitly platform-specific, feature work is complete only when both implementations and their tests are updated.

## Architecture

The main pipeline is:

1. User input is parsed into a normalized model.
2. The model is built into a render tree.
3. The render tree is queried over time into pattern fragments.
4. Playback converts fragments into MIDI output.

Keep behavior in the layer that owns it:

- Syntax belongs in `grammar.txt` and the parser/evaluator.
- Musical transformations belong in renderer nodes and pattern utilities.
- Scheduling belongs in the engine/player.
- Device-specific output belongs in the playback device.

The JS integration point is `core/js/application.js`. Avoid adding feature logic there unless the change is application wiring.

## Feature Workflow

For new syntax:

1. Update `grammar.txt`.
2. Mirror the grammar in `core/cpp/src/parser/KrillGrammar.hpp`.
3. Update C++ semantic actions in `core/cpp/src/parser/KrillParser.cpp`.
4. Add JS and C++ parser tests.
5. Regenerate AST snapshots when the normalized model intentionally changes.

For new render behavior:

1. Add or update the JS render node under `core/js/renderer/nodes/`.
2. Wire JS dispatch through `core/js/renderer/render-tree.js` and its factories.
3. Add the matching C++ render node under `core/cpp/src/renderer/nodes/`.
4. Wire C++ dispatch through `core/cpp/src/renderer/RenderTreeBuilder.cpp` and its factories.
5. Add focused JS and C++ contract tests.

For user-visible musical behavior, add an end-to-end shared fixture where appropriate. User-facing fixtures belong in `core/test-cases.json`; harness schema cases belong in `core/test-cases-runner.json`.

## Validation

Run the JavaScript suite:

```bash
node test.js
```

Run the full cross-language parity workflow:

```bash
npm run test-parity-contract-all
```

See [BUILD-AND-TEST.md](BUILD-AND-TEST.md) for the C++ and Windows workflow.

## Review Checklist

- Both JS and C++ implementations are aligned.
- Parser and render behavior have focused tests.
- Existing timing behavior is unchanged unless intentionally modified.
- Shared AST snapshots are updated only for intentional model changes.
- Documentation reflects user-visible syntax and architectural changes.
- `git diff --check` passes.
