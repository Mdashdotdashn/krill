# Velocity Implementation Plan

## Goal

Add velocity support to Krill mininotation using `:` syntax while keeping the runtime compatible with a future multi-stream architecture.

## Delivery Rule

A story is not complete until both implementations are updated and validated:

- JavaScript implementation
- C++ implementation
- JavaScript tests
- C++ tests

Passing only one language is not sufficient for feature completion in this repository.

## Agreed Semantics

- Syntax: `step:VALUE`
- Default velocity: `127`
- Integer values: absolute velocity
- Float values in `[0,1]`: multiplier of the default/base velocity
- Group velocity: multiplies child/default velocity
- Final velocity `0` is allowed
- Runtime selector/control name: `velocity`

## Epic 1: Syntax And Parsing Foundation

### Story 1.1: Define velocity mininotation semantics

- Support `bd:100`, `sd:80`, `hh:0.8`
- Support nested and grouped forms such as `[bd sd]:100`
- Preserve existing modifier behavior for `@`, `%`, `*`, `/`, and bjorklund syntax

### Story 1.2: Implement parser support in JS and C++

- Extend shared grammar in `grammar.txt`
- Mirror grammar in `core/cpp/src/parser/KrillGrammar.hpp`
- Add C++ semantic action in `core/cpp/src/parser/KrillParser.cpp`
- Keep AST parity between JS and C++

### Acceptance Criteria

- Grammar parses velocity suffixes in both runtimes
- Existing grammar behavior remains intact
- AST parity tests stay green after snapshot refresh

### Status

- Implemented
- JS tests passed
- C++ tests passed

## Epic 2: Event Model And Runtime Controls Path

### Story 2.1: Introduce optional control metadata on runtime events/fragments

- Add optional `controls` payload to runtime query fragments
- Preserve existing `value` and timing behavior when controls are absent

### Story 2.2: Preserve control metadata through render-node composition

- Propagate controls through element, horizontal, vertical, and timeline query paths
- Ensure timing is unchanged relative to pre-velocity behavior

### Acceptance Criteria

- Events without controls behave exactly as before
- Events with `controls.velocity` survive composition
- Query timing/output shape stays stable aside from optional controls metadata

### Status

- JS runtime path implemented
- C++ runtime path implemented
- JS tests passed
- C++ tests passed
- Current status: complete

## Epic 3: MIDI Output And Conversion Behavior

### Story 3.1: Resolve velocity values consistently

- Omitted velocity -> `127`
- Integer velocity -> absolute value
- Float velocity in `[0,1]` -> multiplier of base/default velocity
- Group velocity composes multiplicatively with child/default velocity

### Story 3.2: Use event velocity in live MIDI and MIDI-file export

- Replace hardcoded note-on velocity with per-note velocity
- Preserve velocity in exported MIDI notes

### Acceptance Criteria

- Live MIDI note-on messages use expected velocity values
- MIDI export uses expected velocity values
- Existing playback without velocity annotations remains unchanged

### Status

- Story 3.1 complete: inherited velocity now resolves through a separate `velocityFactor` in the shared render contract.
- Story 3.2 complete: JS live MIDI and MIDI-file export use the resolved per-note velocity.
- JS tests passed.
- C++ tests passed.

## Epic 4: Test Infrastructure Modernization

### Story 4.1: Extend shared run-case schema to support optional controls

- Preserve legacy format:
  - `time -> ["bd", "sd"]`
- Add explicit format:
  - `time -> [{"value":"bd","controls":{"velocity":100}}]`

### Story 4.2: Support readable shorthand in fixtures

- Allow compact entries such as `"bd:100"`, `"sd:80"`, `"hh:0.8"`
- Normalize shorthand internally to value plus controls for comparison

### Story 4.3: Expand regression coverage

- Add parser cases
- Add propagation cases
- Add runtime output cases

### Status

- Deferred by request until additional hands-on usage feedback clarifies target scenarios.

### Acceptance Criteria

- Legacy fixtures continue to pass unchanged
- Explicit object form can assert velocity
- Shorthand and object form can be mixed in one fixture set
- JS and C++ harnesses compare normalized results consistently

## Epic 5: Stream-Compatibility Guardrails

### Story 5.1: Keep velocity per-event and selector-addressable

- Do not introduce global velocity state
- Keep runtime data model stream-agnostic

### Story 5.2: Preserve compatibility posture for future import modes

- Keep `:` as canonical Krill syntax for now
- If needed later, add compatibility/import mode for Strudel/Tidal syntax differences

### Acceptance Criteria

- Velocity remains attached to individual events
- Future stream operations can target `velocity` by control name

### Status

- Story 5.1 complete: velocity remains per-event and no global velocity state was introduced.
- Story 5.2 complete for current scope: `:` remains canonical Krill syntax and compatibility-mode work is explicitly deferred.
- JS and C++ guardrail tests now assert sibling event isolation for velocity controls.

## Epic 6: Documentation And Release Readiness

### Story 6.1: Update user-facing docs

- Document syntax, defaults, group behavior, and examples

### Story 6.2: Update architecture docs

- Describe parse -> fragment controls -> playback flow

### Story 6.3: Final validation

- JS tests green
- C++ tests green
- Parity checks green
- Manual MIDI verification complete

### Status

- Story 6.1 complete: user-facing velocity syntax and semantics documented.
- Story 6.2 complete: architecture flow updated with parse -> fragment controls -> playback velocity resolution details.
- Story 6.3 in progress: automated validation items are tracked; manual MIDI verification remains a final release-gate step.

## Cross-Language Completion Policy

For this workstream, every development item must be implemented and tested in both languages unless it is explicitly documented as platform-specific. That means:

- parser changes must land in JS and C++
- renderer/event model changes must land in JS and C++
- runtime expectation changes must be asserted in JS and C++ tests
- plan status must distinguish between fully complete and JS-only partial progress

## Notes On Strudel Compatibility

- Strudel exposes `velocity`/`vel` as a named control in the `0..1` range
- Strudel also uses `:` in multiple existing syntactic contexts
- This does not block the Krill plan
- If portability becomes important later, add an import/compatibility layer rather than changing the runtime control model

## Current Scope Boundaries

Included now:

- `:` velocity syntax
- Default `127`
- Integer absolute and float multiplier forms
- Stream-ready per-event control metadata
- Readable test fixture shorthand like `"bd:100"`

Excluded for now:

- Full multi-stream runtime (`d1`/`d2`/`d3` style model)
- Generic mininotation syntax for all controls
- Stream-level mute/solo/panic workflow

## Future Exploration: Generic Control Operator

The next control-oriented design direction is intentionally recorded here for
later exploration, without changing the current implementation yet.

### Decision

- Add a generic `control` operator as the canonical mechanism for applying a
  pattern of control values to a musical source.
- Use the operator form `control <controlName> <controlPattern> $ <source>`.
- Route colon-based velocity syntax through the same control mechanism under
  the hood where practical, rather than creating a separate velocity-only
  runtime path.
- Keep the runtime data model per-event and stream-agnostic.

### Intended Example

```text
control velocityFactor "[1 0.5 0.3]%3" $ "[hh hh hh hh]%4"
```

This should combine a four-event hi-hat pattern with a three-event repeating
factor pattern, sampling the control pattern at each musical event onset.

### Proposed Stories

1. **Grammar and AST**: add the generic control operator in JS and C++, with
  parity-preserving AST fields for control name, control pattern, and source.
2. **JS render integration**: add the JS control factory, dispatch, and render
  node.
3. **C++ render integration**: add the matching C++ factory, dispatch, and
  render node.
4. **Sampling and merge semantics**: sample controls at event `wholeStart`,
  attach the result to fragment controls, and define deterministic conflict
  precedence for duplicate control names.
5. **Colon integration**: preserve scalar colon behavior while allowing a
  patterned colon form to use the generic control path.
6. **Velocity strictness**: keep absolute `velocity` at leaf events and use
  normalized `[0,1]` `velocityFactor` for patterned/intermediate modulation.
7. **Parity and regression coverage**: test parser shape, 3-against-4 timing,
  sibling isolation, nested controls, and unchanged scalar velocity behavior
  in both JS and C++.
8. **Documentation**: document the compact user-facing form and the
  parse-to-playback control flow.

### Open Semantics To Resolve Before Implementation

- Whether a missing control sample defaults to `1.0`, leaves the event
  unchanged, or suppresses the event.
- How multiple control samples at the same onset are resolved.
- Whether inner/nearest controls override outer controls for the same key.
- Whether patterned colon syntax is worth adding immediately or should follow
  the explicit `control` operator first.

### Explicit Non-Goals For This Exploration

- No `velmod` alias yet; `control` is the proposed canonical command.
- No global velocity state.
- No multi-stream scheduler redesign.
- No generic arbitrary-control mini-notation until control sampling semantics
  are proven useful with velocity.