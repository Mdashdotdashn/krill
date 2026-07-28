# Standalone App Restore Plan

Restore the standalone JavaScript application around the current query-based renderer and player, while keeping JS/C++ timing semantics aligned.

## Scope

- Restore browser/server standalone flow.
- Restore offline script entrypoints.
- Restore unsynced and synced engine paths.
- Preserve query renderer and player seam.

## Core Contract

1. Scheduler call returns a time, not an event payload.
- Preferred name: `nextOnsetTimeFrom(time)`.
- Compatibility name: `advance(time)`.

2. Payload call returns values at a time.
- Preferred name: `eventsAtTime(time)`.
- Compatibility name: `eventForTime(time)`.

3. Windowed scheduling.
- `nextOnsetTimeFrom(time)` queries a fixed-length window starting at `time`.
- Window size is user-configurable.
- No indefinite scanning.
- If no onset is found in the current window, `eventsAtTime`/`eventForTime` at that candidate time returns `[]`, and the next scheduler call moves to the next window.

4. Exact time arithmetic everywhere.
- Any stored, accumulated, compared, or emitted logical time must use exact rational fractions.
- No floating-point accumulation for player or engine timelines.

5. Cross-runtime parity.
- The exact-time rule and scheduler semantics are mandatory in both JS and C++.

6. Window cache.
- Cache recent queried windows (fraction-normalized start/end + onset index) to avoid over-querying during busy cycles.
- Cache keys and comparisons must use normalized rational values.
- Invalidate cache on tree swap, reset, and window/epsilon configuration changes.

## Implementation Phases

1. Bootstrap and offline seams
- Keep `evaluateRenderingTree` available for standalone scripts.
- Keep standalone `render.js` and `explore.js` functional.

2. Browser/server restore
- Keep `/command` behavior stable.
- Keep `/reporter` polling contract stable.
- Keep bounded reporter buffer with drain semantics.

3. Engine runtime restore
- Unsynced path: schedule from `nextOnsetTimeFrom`/`advance` and emit from `eventsAtTime`/`eventForTime`.
- Synced path: same event semantics, transport-specific timing source.

4. API naming rollout
- Add preferred names (`nextOnsetTimeFrom`, `eventsAtTime`).
- Keep compatibility names (`advance`, `eventForTime`).
- Migrate call sites/tests to preferred names progressively.

5. Tests and verification
- Focused endpoint tests for `/reporter` drain and command error path.
- Player state-machine tests for monotonic scheduling and boundary swap.
- Query contract tests and run-cases checks.
- Sync transition checks (start/clock/stop).

6. Legacy script cleanup
- `render.js` and `explore.js` may be deleted if they are still present, once equivalent standalone or app-driven flows are verified.

## File Focus

- `js/playback/rendering-tree-player.js`
- `js/playback/engine.js`
- `js/playback/sync-device.js`
- `js/application.js`
- `main.js`
- `tests/test-player-state-machine.js`
- `tests/test-reporter.js`

## Non-Negotiable Boundaries

- Keep the current query renderer architecture.
- Do not reintroduce float-based timeline accumulation.
- Do not couple scheduler-time lookup with payload extraction.

## Agile Story Backlog

### Epic A: Standalone Runtime Restoration

#### Story A1: Restore offline evaluator seam
As a tools developer, I want `evaluateRenderingTree` to remain available to standalone scripts, so that offline utilities continue to work without the server runtime.

Acceptance criteria:
- `render.js` can call `evaluateRenderingTree` and produce output without starting the browser/server app.
- `explore.js` can call `evaluateRenderingTree` for inspect-style workflows.
- Existing script behavior is unchanged for valid inputs.
- Script error output remains actionable (non-zero exit on failure plus useful message).

#### Story A2: Keep standalone entrypoints operational
As a user running local script flows, I want standalone entrypoints to stay functional, so that CI and local automation do not regress while app flows are restored.

Acceptance criteria:
- `render.js` executes successfully against representative input.
- `explore.js` executes successfully against representative input.
- Any changed imports/exports are backward-compatible or explicitly shimmed.

#### Story A3: Remove legacy standalone scripts when replacement flows are proven
As a maintainer, I want to delete legacy standalone scripts when equivalent flows are covered, so that the codebase avoids redundant entrypoints.

Acceptance criteria:
- If `render.js` and `explore.js` are still present, they can be removed after replacement flows are verified.
- Equivalent behavior is covered by maintained entrypoints and tests before deletion.
- CI and local developer workflows remain documented and functional after removal.

### Epic B: Browser/Server Contract Restoration

#### Story B1: Preserve `/command` behavior
As a browser client, I want `/command` behavior to remain stable, so that command dispatch and error handling remain predictable.

Acceptance criteria:
- Existing `/command` success responses remain backward-compatible.
- Existing `/command` error shape remains backward-compatible.
- Regression tests cover at least one success and one error path.

#### Story B2: Preserve `/reporter` polling and drain semantics
As a polling client, I want `/reporter` to preserve drain semantics with bounded buffering, so that I can consume events reliably without memory growth.

Acceptance criteria:
- `/reporter` returns queued reports and drains consumed items.
- Reporter queue is bounded and does not grow unbounded under load.
- Polling with no new reports returns an empty result in the existing shape.
- Tests verify both drain behavior and buffer limit behavior.

### Epic C: Scheduler/Payload API Contract

#### Story C1: Separate scheduling from payload retrieval
As a playback engine maintainer, I want scheduler-time lookup and payload extraction to be separate calls, so that timing and event retrieval remain decoupled and testable.

Acceptance criteria:
- Scheduler API returns time only (`nextOnsetTimeFrom` and compatibility `advance`).
- Payload API returns events only (`eventsAtTime` and compatibility `eventForTime`).
- No code path returns both next time and payload from a single call.
- Unit tests verify separation in both unsynced and synced usage paths.

#### Story C2: Roll out preferred API names without breaking compatibility
As a codebase maintainer, I want preferred names available while keeping compatibility names, so that migration can happen incrementally without breaking existing callers.

Acceptance criteria:
- Preferred names are implemented: `nextOnsetTimeFrom`, `eventsAtTime`.
- Compatibility names are retained: `advance`, `eventForTime`.
- Existing callers continue to work before migration.
- New/updated tests prefer the new names.

### Epic D: Windowed Scheduling and Cache

#### Story D1: Implement bounded windowed scheduling
As a playback engine maintainer, I want scheduling to query fixed windows, so that next-onset search is bounded and predictable.

Acceptance criteria:
- `nextOnsetTimeFrom(time)` queries only a fixed-size window from `time`.
- Window size is user-configurable.
- No indefinite scan loop exists in scheduler logic.
- If no onset exists in the current window, payload query at the candidate time returns `[]`.
- The next scheduler call advances to the next window.

#### Story D2: Add normalized rational window cache
As a performance-focused maintainer, I want recent windows cached using normalized rational keys, so that busy cycles avoid redundant queries without semantic drift.

Acceptance criteria:
- Cache key includes normalized rational window start/end and onset index metadata.
- Cache lookup uses normalized rational comparison, not floating-point approximation.
- Cache invalidates on tree swap.
- Cache invalidates on reset.
- Cache invalidates on window-size or epsilon configuration change.

### Epic E: Exact-Time Semantics and Cross-Runtime Parity

#### Story E1: Enforce exact rational timeline arithmetic in JS playback
As an engine developer, I want all JS logical timeline operations to use exact rational arithmetic, so that accumulated timing error is eliminated.

Acceptance criteria:
- Stored and accumulated timeline values use rational/fraction types.
- Time comparisons use exact rational semantics.
- Emitted logical times are rational-derived and deterministic.
- No float-based accumulation remains in player/engine timeline code paths.

#### Story E2: Enforce equivalent exact-time and scheduler semantics in C++
As a cross-runtime maintainer, I want C++ runtime behavior to mirror JS exact-time and scheduling semantics, so that parity is preserved across implementations.

Acceptance criteria:
- C++ scheduling uses the same scheduler/payload split contract.
- C++ timeline arithmetic follows exact-time semantics equivalent to JS.
- Parity checks cover representative scheduler boundary cases.

### Epic F: Engine Flow and Verification

#### Story F1: Restore unsynced engine path on new contract
As a standalone runtime user, I want unsynced playback to run with the scheduler/payload split, so that local playback behavior is stable and deterministic.

Acceptance criteria:
- Unsynced path schedules via `nextOnsetTimeFrom`/`advance`.
- Unsynced path emits via `eventsAtTime`/`eventForTime`.
- Scheduling remains monotonic through normal progression.
- Tests cover boundary swap behavior.

#### Story F2: Restore synced engine path with transport-specific clock
As a synchronized playback user, I want synced playback to use the same event semantics while honoring transport timing, so that start/clock/stop flows are reliable.

Acceptance criteria:
- Synced path uses same scheduler/payload semantics as unsynced path.
- Timing source is transport/clock specific, not ad hoc.
- Start/clock/stop transitions are covered by tests.

#### Story F3: Complete regression and contract verification suite
As a maintainer, I want focused automated verification for restored flows, so that regressions are caught early.

Acceptance criteria:
- `/reporter` drain and `/command` error-path tests are in place.
- Player state-machine tests cover monotonic scheduling and boundary swap.
- Query contract tests pass.
- Existing run-cases checks pass.
- Sync transition checks pass.

## Suggested Delivery Order

1. A1, A2, A3 (stabilize standalone seams and retire legacy scripts when safe)
2. B1, B2 (stabilize external HTTP contracts)
3. C1, C2 (codify API contract and naming migration)
4. D1, D2 (bounded scheduling and performance safety)
5. E1, E2 (exact-time enforcement and cross-runtime parity)
6. F1, F2, F3 (runtime restore completion plus verification)
