# Possible Future Development

This document collects ideas that may be useful in the future. It is not a committed roadmap or delivery schedule. Items can be revisited when musical experiments, user feedback, or architectural needs make them relevant.

## Core Improvements

- Constant-fold render nodes when their children are constant.
- Normalize factory argument construction so every argument is built consistently and arity is checked.
- Strengthen `stack` behavior and add focused coverage for parallel sequences with operators.

Example:

```text
stack [ "hh hh", fast 2 $ "bd sd" ]
```

## Language And Pattern Ideas

- Accent and ghost-note tokens, with syntax still to be decided.
- A user-provided JavaScript transform hook for experimental pattern manipulation.
- More expressive `stack` grouping for parallel processing.
- Improvements to `add` for octave-shift and related workflows.
- Shift as a pattern modifier.
- More compact alternation, repetition, and polymeter-like mininotation.

## Tidal/Strudel-Inspired Ideas

- Independent live-coding streams such as `d1`, `d2`, and `d3`, including mute, solo, hush, panic, and per-stream replacement.
- Higher-order periodic transforms such as `every`, `whenmod`, `jux`, and `off`.
- Deterministic randomness tools such as `choose`, `sometimes`, `degrade`, `rand`, and `irand`, with explicit seed control.
- Pattern-valued operator parameters for timing, pitch transformations, and event controls.

## Transitions

These ideas are intentionally framed for Krill's note-event model rather than audio synthesis:

- Event-level crossfades that blend note patterns over a cycle window.
- A defined collision policy when two patterns emit notes at the same time.
- Scene and replacement transitions, including hard switches and optional blend windows.

## Performance And Observability

- Timing diagnostics for jitter, latency, and late events.
- A lightweight event inspector showing current query output and decision paths.
- Separate control or macro lanes for hardware controllers, with reusable mapping presets.

## Reference Ideas

These examples are starting points for future exploration, not currently supported syntax:

```text
d1 $ every' 8 0 (const (s "cc")) $ silence
d1 $ jux (slow 1.005) $ s "its_gonna_rain"
```

## Related Design Notes

- [Parameter modulation design](PARAMETER-MODULATION-DESIGN.md)
- [Architecture](ARCHITECTURE.md)
