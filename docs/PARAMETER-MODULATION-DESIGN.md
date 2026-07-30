# Parameter Modulation Design

## Overview

This document describes the design for supporting external parameter injection (knobs, MIDI CC, normalized control values) into Krill pattern operators—enabling a VCV Eurorack module or similar to modulate pattern behavior in real time.

## Problem Statement

When wrapping Krill's C++ core into a eurorack module, knob inputs need to modulate pattern operators. Unlike static operators:

```
slow 2 $ "bd hh sd"          // fixed: always slow 2x
slow [1 0.5 1 0.25] $ "..."  // temporal: pattern changes over time
```

We need a way to inject normalized (0–1) external values as **dynamic patterns** that time-align with content patterns.

## Design: Option B Syntax

### External Parameter References

Reserved names `%p1` through `%p16` refer to external parameters (auto-filled by the system):

```
slow %p1 $ "bd hh sd"
scale %p2 "major" $ "0 2 4 6"
struct %p3 $ "bd"
fast %p4 $ "hh hh hh"
```

Visual distinction: The `%` prefix immediately signals "this comes from outside" (knobs, MIDI CC, hardware input).

### User-Defined Patterns

Plain identifiers require explicit binding via `let`:

```
let mySpeed = "[1 0.5 1 0.25]"
let tempo = "[0.25 0.5 1 2]"

slow mySpeed $ "bd hh sd"
fast tempo $ "hh*8"
```

## Rationale

### Why `%p1` Instead of Alternatives?

| Syntax | Pros | Cons | Rejected? |
|--------|------|------|-----------|
| `$k1` | Short | Conflicts with `$` pipe operator | ✓ |
| `^p1` | Mathematical-looking | Conflicts with exponentiation operator | ✓ |
| `{p1}` | Braces clear | Conflicts with Tidal polymeter syntax | ✓ |
| `#p1` | Music notation familiar | Conflicts with channel notation `#1` | ✓ |
| `p1` (plain) | Simple, Tidal-like | No visual distinction; scope confusion | ✓ |
| **`%p1`** | **Clear distinction; no conflicts** | **Slightly verbose** | ✓ Chosen |

### Scope Model

**Session-global**: All patterns see the same `%p1` value at any given time. This enables:
- Synchronized modulation across multiple playback sequences
- Knob/CC changes affecting all active patterns uniformly
- Simple mental model: "CC#1 is CC#1 everywhere"

Future work could support pattern-scoped parameters if parallelism requires decoupling.

## Implementation Architecture

### Four-Layer Pipeline

1. **Grammar** (`grammar.txt`)
   - Add rule: `parameter_reference ← '%' identifier`
   - Extend `operator_argument` to include parameter_reference

2. **Parser** (C++ and JS)
   - Recognize `%identifier` tokens
   - Create `ParameterReference` AST node type
   - Store parameter name for later resolution

3. **Renderer**
   - Operator nodes accept `RenderNodePtr` for dynamic parameters
   - Example: `SlowRenderNode` can hold either:
     - Static `Fraction mFactor` (current)
     - Or `RenderNodePtr mpDynamicFactor` (new)
   - Build factories instantiate parameter patterns and pass to operators

4. **Query/Playback**
   - When querying: sample parameter pattern + content pattern in sync
   - Reuse existing weaving logic for pattern intersection
   - Apply operator transformation per event using sampled parameter value

### Affected Operator Nodes

All operators with numeric/structural parameters:

- `SlowRenderNode` — time scaling factor
- `StretchRenderNode` — stretch factor
- `ShiftRenderNode` — time offset
- `TruncRenderNode` — duration
- `BjorklundRenderNode` — pulses/steps (if dynamic)
- `StructRenderNode` — structure pattern (if allowing parameter weaving)
- `ScaleRenderNode` — scale selection (requires string/enum handling)

### Files to Modify

**Grammar & Parser:**
- `grammar.txt`
- `core/cpp/src/parser/KrillParser.cpp`
- `core/js/input-evaluator.js`

**Renderer:**
- `core/cpp/src/renderer/nodes/*.hpp` (operator nodes)
- `core/cpp/src/renderer/factories/OperatorNodeFactory.cpp`
- `core/js/renderer/render-tree.js`
- `core/js/renderer/nodes/*.js` (JS operator nodes)

**Docs:**
- `docs/ARCHITECTURE.md` — Add parameter injection section

## Example Usage

```
// Simple knob modulation
setcc 1 as p1          // Map MIDI CC#1 to parameter p1

slow %p1 $ "bd hh sd"  // Slow factor follows CC#1 (0–1 normalized)
fast %p1 $ "hh*16"     // Speed up/down with same CC
scale %p1 "major" $ "0 2 4 6"  // Different scales based on CC

// Mixed static + dynamic
slow 2 $ scale %p1 "major" $ "bd hh"  // Static 2x slow, dynamic scale

// Composition with let
let basePattern = "bd [sd hh] bd sd"
let speed = %p1
slow speed $ basePattern

let modulatedSpeed = "slow %p1"  // NOT ALLOWED—parameters only in operator arguments
```

## Relationship to Tidal

This mirrors Tidal's **pattern composition** approach:

- Tidal: `(slow <*> speeds) $ pattern` — apply pattern of operators to pattern
- Krill: `slow %p1 $ pattern` — syntactic sugar where `%p1` is a pattern source
- Both: parameters are first-class patterns with time-varying values

Key difference: Krill's `%` prefix makes external vs. local parameters explicit at the call site.

## Backward Compatibility

- No impact on existing patterns (no `%` usage today)
- `let` bindings unchanged
- Existing operators continue to accept static numeric arguments
- Purely additive feature

## Future Extensions

1. **Pattern-scoped parameters**: Allow `slow %p1 $ "bd"` and `slow %p2 $ "hh"` to use different values
   - Requires context propagation through render tree
   - Useful for parallel, independent sequences

2. **Parameter arithmetic**: `slow (%p1 + 0.5) $ "bd"` — combine parameters with constants
   - Requires expression evaluation in operator arguments
   - Deferred to v2+

3. **Polymeter support**: If Krill adds `{a, b}` polymeter syntax, ensure it doesn't conflict with parameter syntax ✓ (already using `%`)

4. **Named parameter sets**: `setparams "drum" = {p1, p2, p3}` for complex hardware layouts

## Status

**Design**: Complete (2026-07-30)
**Implementation**: Shelved—syntax frozen, awaiting priority

---

*See [ARCHITECTURE.md](ARCHITECTURE.md) for general system overview.*
