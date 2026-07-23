# Interactive Player Restore Plan

Re-enable live JS playback on top of the rebuilt query renderer by restoring a stateful playback adapter, wiring a real engine clock, and exposing server-side event reporting for the browser poll loop. The recommended sequence is unsynced clock first, then optional MIDI sync, while preserving atomic queue and replace at cycle boundaries.

## Steps

1. Phase 1 - Lock requirements and baseline behavior.
2. Confirm command contract in [js/application.js](../js/application.js): input string parsing, tempo control command shape, hush semantics, and expected queue behavior when a new pattern arrives mid-cycle.
3. Capture the current non-working interactive path with a focused smoke test checklist against [main.js](../main.js), [js/application.js](../js/application.js), and [public/index-edit.html](../public/index-edit.html). This creates a before-and-after validation baseline.
4. Phase 2 - Restore player state machine on query API.
5. Rebuild [js/playback/rendering-tree-player.js](../js/playback/rendering-tree-player.js) from query-only shim into a stateful adapter with reset, clear, advance, and eventForTime while keeping queryArc as the source of event fragments.
6. Implement queued tree replacement in [js/playback/rendering-tree-player.js](../js/playback/rendering-tree-player.js): setRenderingTree stores pending tree; swap occurs only at cycle boundary. This step depends on step 5.
7. Preserve deterministic event ordering and cycle offsets by deriving next event times from query fragments instead of reintroducing legacy render callbacks. This step depends on step 5.
8. Phase 3 - Re-enable unsynced engine timing and event emission.
9. Implement unsynced scheduling loop in [js/playback/engine.js](../js/playback/engine.js): start, stop, processUnsyncedEvent, and processPlayerEvent should convert cycle deltas to wall-clock delays using cps.
10. Keep command-driven tempo changes active via [js/application.js](../js/application.js), and add a bpm-friendly alias if needed so interactive usage can set tempo directly. This step depends on step 9.
11. Ensure hush immediately stops outgoing notes and clears queued playback state by coordinating [js/playback/engine.js](../js/playback/engine.js), [js/application.js](../js/application.js), and [js/playback/playback-device.js](../js/playback/playback-device.js). This step depends on steps 6 and 9.
12. Phase 4 - Restore interactive server reporting.
13. Implement the missing reporter endpoint in [main.js](../main.js) and back it with a bounded in-memory event buffer fed by engine ticks.
14. Keep the existing polling model in [public/index-edit.html](../public/index-edit.html) and make /reporter return structured event batches with timestamps and payload values. This step depends on step 13.
15. Wire application tick flow so both playback device output and reporter stream observe the same event objects, avoiding divergent behavior between audible output and UI logs. This step depends on steps 9 and 13.
16. Phase 5 - Add targeted tests and regression gates.
17. Add player seam unit tests for queue-and-replace boundary behavior, eventForTime matching, and advance monotonicity in a new JS test file under [tests](../tests).
18. Add engine timing tests for cps-to-delay conversion and hush behavior, using deterministic clock fakes where needed. This step can run in parallel with step 17.
19. Add server integration smoke test: submit command to /command, poll /reporter, assert event delivery order and payload shape. This step depends on steps 13 to 15.
20. Run full regression gates including [tests/test-parity-contract-all.sh](../tests/test-parity-contract-all.sh) to confirm interactive restoration does not regress renderer parity. This step depends on steps 17 to 19.
21. Phase 6 - Optional MIDI sync pass.
22. Implement sync-device wiring in [js/playback/engine.js](../js/playback/engine.js) and [js/playback/sync-device.js](../js/playback/sync-device.js) only after unsynced path is green.
23. Add sync-specific tests and hardware smoke notes; keep this phase isolated to avoid blocking unsynced release. This step depends on step 20.

## Relevant files

- [js/playback/rendering-tree-player.js](../js/playback/rendering-tree-player.js) - restore stateful player seam and queue/replace boundary swap.
- [js/playback/engine.js](../js/playback/engine.js) - implement actual scheduler loop, cps timing, hush, optional sync hooks.
- [js/application.js](../js/application.js) - command handling, tree submission, tick routing to output and reporter.
- [main.js](../main.js) - add reporter endpoint and server-level event plumbing.
- [public/index-edit.html](../public/index-edit.html) - existing polling client for /reporter; validate compatibility.
- [js/playback/playback-device.js](../js/playback/playback-device.js) - output sink behavior for tick and hush.
- [js/playback/sync-device.js](../js/playback/sync-device.js) - optional second-pass sync wiring.
- [render.js](../render.js) - verify offline player flow works with restored player methods.
- [tests/test-run-cases.js](../tests/test-run-cases.js) - keep renderer behavior checks intact.
- [tests/test-parity-contract-all.sh](../tests/test-parity-contract-all.sh) - final parity gate.

## Verification

1. Manual interactive smoke: run server, submit pattern via /command, observe recurring timed events through /reporter at the configured tempo.
2. Queue/replace check: submit pattern A, then pattern B mid-cycle, verify B starts at next cycle boundary and A does not leak past swap point.
3. Hush check: issue hush command and verify immediate note-off plus empty subsequent reporter batches.
4. Offline render check: run [render.js](../render.js) with a known sequence and confirm output file generation still works.
5. Automated tests: run new player and engine tests, then run [tests/test-parity-contract-all.sh](../tests/test-parity-contract-all.sh).

## Decisions

- Included scope: unsynced interactive restore, queue-and-replace boundary semantics, existing /reporter polling model.
- Excluded from first pass: mandatory MIDI sync completion; this is intentionally phased after unsynced path is green.
- Swap policy: new render trees are queued and applied atomically at cycle boundaries.

## Further considerations

1. Tempo command UX: keep cps as internal unit but add bpm command alias for interactive users.
2. Reporter payload contract: define stable JSON schema now to avoid UI drift while extending telemetry later.
3. Timer precision: choose monotonic clock source and drift correction strategy before sync phase.
