# Contributing

This file is for contributor-facing notes. The top-level README should stay focused on installation, usage, and the language from a user's point of view.

For embedded-specific architecture notes, see [embedded/readme.md](embedded/readme.md).

## JS core principles

The JavaScript side of Krill is organized as a pipeline:

1. user input is parsed into a normalized model
2. that model is turned into a rendering tree
3. the rendering tree is rendered over time into pattern events
4. playback code emits those events to a device

The important boundary is between the parsed model and the runtime tree. Syntax work belongs in the parser and evaluator. Musical behavior belongs in operators, patterns, and playback structures. Try not to blur those layers.

## Main flow

The main server entry point is [main.js](main.js). It creates the web server and forwards `/command` input to [js/application.js](js/application.js).

[js/application.js](js/application.js) is the integration point for the JS runtime:

- [js/input-evaluator.js](js/input-evaluator.js) parses source text using [grammar.txt](grammar.txt)
- [js/rendering-tree.js](js/rendering-tree.js) turns the parsed model into operator and pattern nodes
- [js/playback/engine.js](js/playback/engine.js) schedules playback in cycle time
- [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js) advances the current tree and returns the next event to emit
- [js/playback/playback-device.js](js/playback/playback-device.js) converts event values into MIDI output

If you are changing behavior, find the layer that actually owns that behavior before editing. Avoid putting feature logic into [js/application.js](js/application.js) unless the change is truly about application wiring.

## Parser and evaluator principles

The parser side is intentionally small:

- [grammar.txt](grammar.txt) defines the accepted language
- [js/input-evaluator.js](js/input-evaluator.js) builds the PEG parser and removes empty parse fields

When adding syntax, keep these rules in mind:

- If the language accepts a new form, start in [grammar.txt](grammar.txt)
- Keep the output model explicit and stable so the renderer can consume it without parser-specific assumptions
- Prefer extending existing node shapes when the concept already exists
- Avoid pushing execution details into the parse model if they belong in operator behavior

The JS parser output is also the bridge toward the embedded implementation, so model changes should be made deliberately.

## Rendering tree and operator principles

[js/rendering-tree.js](js/rendering-tree.js) is the bridge between parsed data and runtime behavior. It recursively walks parsed nodes and dispatches them to operator constructors.

The operator framework lives in [js/operators/operators.js](js/operators/operators.js). Operators follow a simple contract:

- they receive arguments that can be rendered
- `tick()` advances stateful children
- `render()` returns a pattern-like result for the current cycle

Most feature work on the JS side should happen here:

- new transformations usually belong in a new file under [js/operators/](js/operators/)
- new pattern composition behavior should reuse the pattern utilities in [js/patterns/](js/patterns/)
- operator dispatch should be wired in [js/rendering-tree.js](js/rendering-tree.js)

Representative files:

- [js/operators/op-add.js](js/operators/op-add.js) shows a binary operator using pattern weaving
- [js/operators/op-pattern.js](js/operators/op-pattern.js) shows how weighted steps and sequence rendering are built
- [js/patterns/pattern.js](js/patterns/pattern.js) defines the core pattern data structure and timing helpers
- [js/patterns/weaving.js](js/patterns/weaving.js) is the place to look for pattern-combination behavior

## Playback principles

Playback should stay generic.

- [js/playback/engine.js](js/playback/engine.js) owns scheduling and tempo
- [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js) owns cycle-to-cycle rendering and event lookup
- [js/playback/playback-device.js](js/playback/playback-device.js) owns device output

If a feature changes musical meaning, it usually belongs in the parser or operator layer, not in the playback loop. Try not to add feature-specific branches to the engine or tree player unless the feature is fundamentally about scheduling.

## Guidelines for new features

Use this rule of thumb:

- New syntax or notation: update [grammar.txt](grammar.txt) and verify the parsed model through [js/input-evaluator.js](js/input-evaluator.js)
- New operator: add an operator file under [js/operators/](js/operators/), wire it in [js/rendering-tree.js](js/rendering-tree.js), and add tests
- New pattern behavior: extend [js/patterns/pattern.js](js/patterns/pattern.js) or related pattern utilities rather than special-casing callers
- New playback or timing behavior: start in [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js) or [js/playback/engine.js](js/playback/engine.js), but only if the feature is truly about scheduling

Typical operator work usually touches four places:

1. [grammar.txt](grammar.txt) if the syntax is new
2. a new or updated file in [js/operators/](js/operators/)
3. operator dispatch in [js/rendering-tree.js](js/rendering-tree.js)
4. tests in [tests/](tests/)

## Tests

There is no useful `npm test` script at the moment. Use the test files directly.

Useful entry points:

- [tests/base.js](tests/base.js) provides shared evaluator helpers
- [tests/test-evaluator.js](tests/test-evaluator.js) covers parser and model behavior
- [tests/test-operator.js](tests/test-operator.js) covers operator contracts
- [tests/test-pattern.js](tests/test-pattern.js) covers pattern-level behavior
- [tests/test-weaving.js](tests/test-weaving.js) covers pattern combination behavior
- [tests/test-sequence-player.js](tests/test-sequence-player.js) covers playback timing
- [tests/test-run-cases.js](tests/test-run-cases.js) runs the shared musical cases from [tests/test-cases.json](tests/test-cases.json)

For changes that affect user-visible musical behavior, prefer adding or updating an end-to-end case in [tests/test-cases.json](tests/test-cases.json) in addition to narrower unit coverage.

## Where to look first

- [main.js](main.js) for server and request flow
- [js/application.js](js/application.js) for top-level JS wiring
- [grammar.txt](grammar.txt) for syntax ownership
- [js/input-evaluator.js](js/input-evaluator.js) for parse entry
- [js/rendering-tree.js](js/rendering-tree.js) for parsed-model to runtime dispatch
- [js/operators/operators.js](js/operators/operators.js) for operator contract
- [js/patterns/pattern.js](js/patterns/pattern.js) for core pattern behavior
- [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js) for event scheduling within a cycle
- [tests/test-run-cases.js](tests/test-run-cases.js) for end-to-end JS behavior

If a change spans multiple layers, the usual order is: syntax, model shape, operator or pattern behavior, playback implications, tests.