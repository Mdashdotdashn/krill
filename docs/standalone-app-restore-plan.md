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
