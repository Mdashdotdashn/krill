# Krill Codebase Refactoring Analysis

## Executive Summary

Analysis of the Krill JS and C++ codebase reveals **1 critical blocker**, **5 quick wins**, and **3 major structural refactorings** to reduce dead code and duplication while improving maintainability.

**Critical Finding**: `engine.js` calls an undefined method (`eventForTime`) that will crash at runtime during synced MIDI playback.

---

## 🔴 CRITICAL ISSUES

### 1. Blocker: Undefined Method Call in engine.js

**Severity**: CRITICAL (Runtime Error)  
**File**: [js/playback/engine.js](js/playback/engine.js#L134-L142)  
**Location**: `processPlayerEvent()` method

```javascript
Engine.prototype.processPlayerEvent = function()
{
  var event = this.renderingPlayer_.eventForTime(this.currentTime_);  // ❌ NOT DEFINED!
  if (event && event.values && event.values.length > 0)
  {
    this.emit("tick", event);
  }
  return event ? event.values.length : 0;
}
```

**Problem**: 
- `eventForTime()` is NOT defined in [rendering-tree-player.js](js/playback/rendering-tree-player.js)
- Only `eventsAtTime()` and `nextOnsetTimeFrom()` exist (the new API)
- The synced playback path is dead code that will throw `TypeError` at runtime
- This is called when `processSyncedEvent()` runs (lines 124-131)

**Impact**:
- Synced MIDI devices (e.g., hardware clock, sync input) will crash the engine
- Unsynced path works because it uses correct API: `eventsAtTime()` + `nextOnsetTimeFrom()` (lines 107-120)

**Fix** (Choose one):
- **Option A** (Quick): Replace `eventForTime()` with `eventsAtTime()` (changes line 136)
- **Option B** (Clean): Remove synced path entirely if unused and document rationale

**Estimate**: 5 minutes

---

## 🟡 QUICK WINS (Low Effort, Clear Benefit)

### 1. Remove Unused Lodash Import

**File**: [js/input-evaluator.js](js/input-evaluator.js#L3)  
**Issue**: Declares `const _ = require('lodash');` but never uses it

```javascript
const _ = require('lodash');  // ❌ Unused

const removeEmpty = e => e instanceof Object ? Object.entries(e).reduce((o, [k, v]) => {
  // ✓ Does not use lodash, implements its own reduce
  if (typeof v === 'boolean' || v) o[k] = removeEmpty(v);
  return o;
}, e instanceof Array ? [] : {}) : e;
```

**Fix**: Delete line 3  
**Estimate**: 1 minute

---

### 2. Add Missing API Documentation

**File**: [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js)  
**Issue**: Mix of old and new API names without clear guidance

```javascript
// Current state (confusing):
// - eventsAtTime() and nextOnsetTimeFrom() exist (NEW, preferred)
// - No advance() or eventForTime() defined (but engine.js tries to use them)
```

**Fix**: Add comments explaining the API transition:

```javascript
// Preferred API (new, do not use old names):
// - nextOnsetTimeFrom(time) → returns Fraction (next onset time)
// - eventsAtTime(time) → returns string[] (values at time)
//
// Deprecated API (being removed):
// - advance() → was an alias for nextOnsetTimeFrom()
// - eventForTime() → was an alias for eventsAtTime()
//
// See: docs/standalone-app-restore-plan.md Phase 4
```

**Estimate**: 5 minutes

---

### 3. Consolidate Synced/Unsynced Event Logic in Engine

**File**: [js/playback/engine.js](js/playback/engine.js#L100-L142)  
**Issue**: Three methods (`processUnsyncedEvent`, `processSyncedEvent`, `processPlayerEvent`) are 85% identical

```javascript
// Current (duplicated logic):
Engine.prototype.processUnsyncedEvent = function() {
  var values = this.renderingPlayer_.eventsAtTime(this.currentTime_);
  if (values && values.length > 0) {
    this.emit("tick", {time: this.currentTime_, values: values});  // Emit tick
  }
  var nextTime = this.renderingPlayer_.nextOnsetTimeFrom(this.currentTime_);
  var deltaCycles = math.subtract(nextTime, this.currentTime_);
  var delayMs = Math.max(1, Math.round((math.number(deltaCycles) * 1000) / this.cps_));
  this.currentTime_ = nextTime;  // Schedule next
  // ...
}

Engine.prototype.processSyncedEvent = function() {
  if (!this.running_ || !this.syncOn_) return;
  this.processPlayerEvent();  // Delegates to...
}

Engine.prototype.processPlayerEvent = function() {
  var event = this.renderingPlayer_.eventForTime(this.currentTime_);  // ❌ Broken!
  if (event && event.values && event.values.length > 0) {
    this.emit("tick", event);  // Duplicate emit logic
  }
}
```

**Pattern**: Both emit event, then schedule next. The synced path is also broken (uses undefined method).

**Fix**: Refactor to single method:

```javascript
Engine.prototype.processEvent = function() {
  if (!this.running_) return;
  
  var values = this.renderingPlayer_.eventsAtTime(this.currentTime_);
  if (values && values.length > 0) {
    this.emit("tick", {time: this.currentTime_, values: values});
  }
  
  var nextTime = this.renderingPlayer_.nextOnsetTimeFrom(this.currentTime_);
  this.currentTime_ = nextTime;
  
  if (this.synced_) {
    this.processSyncedEvent();
  } else {
    this.scheduleNextUnsyncedEvent(nextTime);
  }
}

Engine.prototype.scheduleNextUnsyncedEvent = function(nextTime) {
  var deltaCycles = math.subtract(nextTime, this.currentTime_);
  var delayMs = Math.max(1, Math.round((math.number(deltaCycles) * 1000) / this.cps_));
  
  var self = this;
  this.unsyncedTimer_ = setTimeout(function() {
    self.processEvent();
  }, delayMs);
}
```

**Estimate**: 15 minutes

---

### 4. Extract Fraction Utilities

**Files**: [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js), [js/renderer/nodes/*.js](js/renderer/nodes/) (13 files)  
**Issue**: Duplicate fraction helpers scattered across codebase

```javascript
// Current (repeated in multiple files):
RenderingTreePlayer.prototype.toFraction_ = function(value) {
  return math.fraction(value);
}

RenderingTreePlayer.prototype.cycleStart_ = function(time) {
  return math.fraction(math.floor(this.toFraction_(time)));
}

RenderingTreePlayer.prototype.nextCycleBoundary_ = function(time) {
  return math.add(this.cycleStart_(time), math.fraction(1));
}

RenderingTreePlayer.prototype.epsilon_ = function() {
  return math.fraction(1, 1024);
}

// Similar logic duplicated in: shift-render-node.js, other nodes...
```

**Fix**: Create `js/utils/time-utils.js`:

```javascript
var math = require('mathjs');

TimeUtils = {
  toFraction: function(value) {
    return math.fraction(value);
  },

  cycleStart: function(time) {
    return math.floor(TimeUtils.toFraction(time));
  },

  nextCycleBoundary: function(time) {
    return math.add(TimeUtils.cycleStart(time), math.fraction(1));
  },

  epsilon: function() {
    return math.fraction(1, 1024);
  },

  overlapBounds: function(requestStart, requestEnd, wholeStart, wholeEnd) {
    // Consolidate QueryNodeUtils.overlapBounds here
  }
};
```

Then replace private methods:

```javascript
RenderingTreePlayer.prototype.toFraction_ = TimeUtils.toFraction;
RenderingTreePlayer.prototype.cycleStart_ = TimeUtils.cycleStart;
// etc.
```

**Estimate**: 20 minutes

---

### 5. Add Fragment Validation Helper

**Files**: [js/renderer/nodes/*.js](js/renderer/nodes/) (13 files)  
**Issue**: All nodes repeat validation pattern:

```javascript
// Current (repeated ~50 times across nodes):
fragments.forEach(function(fragment) {
  if (fragment.wholeStart === undefined) {
    return;  // Skip invalid fragment
  }
  // Use fragment...
});

if (fragments.length === 0) {
  return [];  // Edge case
}

if (!mpTree) {
  return {};  // Another edge case
}
```

**Fix**: Add to `js/renderer/nodes/query-node-utils.js`:

```javascript
QueryNodeUtils.isValidFragment = function(fragment) {
  return fragment && fragment.wholeStart !== undefined;
};

QueryNodeUtils.filterValidFragments = function(fragments) {
  return (fragments || []).filter(QueryNodeUtils.isValidFragment);
};
```

Then simplify nodes:

```javascript
// Before: ~8 lines of validation
// After:
var validFragments = QueryNodeUtils.filterValidFragments(fragments);
validFragments.forEach(function(f) {
  // Use fragment...
});
```

**Estimate**: 15 minutes

---

## Code Duplication Summary

### JS Renderer Nodes (13 files)

**Pattern**: All follow identical structure with ~40-80 lines each:

| Node Type | File | Lines | Query Pattern |
|-----------|------|-------|--------------|
| Element | [element-render-node.js](js/renderer/nodes/element-render-node.js) | 47 | Basic linear |
| Horizontal Pattern | [horizontal-pattern-render-node.js](js/renderer/nodes/horizontal-pattern-render-node.js) | 85 | Slot division |
| Vertical Pattern | [vertical-pattern-render-node.js](js/renderer/nodes/vertical-pattern-render-node.js) | 35 | Overlap each child |
| Timeline Pattern | [timeline-pattern-render-node.js](js/renderer/nodes/timeline-pattern-render-node.js) | 80 | Weighted distribution |
| Add | [add-render-node.js](js/renderer/nodes/add-render-node.js) | 90 | Merge two sources |
| Bjorklund | [bjorklund-render-node.js](js/renderer/nodes/bjorklund-render-node.js) | 75 | Euclidean rhythm |
| Scale | [scale-render-node.js](js/renderer/nodes/scale-render-node.js) | 68 | Apply note transposition |
| Shift | [shift-render-node.js](js/renderer/nodes/shift-render-node.js) | 75 | Time offset |
| Stretch | [stretch-render-node.js](js/renderer/nodes/stretch-render-node.js) | 43 | Duration scaling |
| Struct | [struct-render-node.js](js/renderer/nodes/struct-render-node.js) | 60 | Mask application |
| Trunc | [trunc-render-node.js](js/renderer/nodes/trunc-render-node.js) | 53 | Duration truncation |
| Empty | [empty-render-node.js](js/renderer/nodes/empty-render-node.js) | 8 | Returns empty |
| Query Utils | [query-node-utils.js](js/renderer/nodes/query-node-utils.js) | 50 | Shared helpers |

**Boilerplate Common to All**:
- Fraction conversion (3-5 lines)
- Null/undefined checks (2-3 lines)
- Overlap calculation (5-8 lines)
- Fragment construction (5-8 lines)
- Total: ~20-25 lines per node of duplicated logic

**Extraction Potential**: 
- Extract base class or mixin: ~200 lines saved
- DRY render node definitions down to core logic
- Add shared tests for common patterns

---

## Medium-Term Refactorings

### 1. Create BaseQueryRenderNode

**Concept**: Extract common query() boilerplate into base class

**Current**: 13 separate implementations of ~40-80 lines each  
**Proposed**: Base class + 13 focused operator implementations of ~10-15 lines each

```cpp
// C++ Already Has This Pattern!
// See embedded/renderer/RenderNode.hpp - uses virtual query() method

// Should mirror in JS:
BaseQueryRenderNode = function() {};
BaseQueryRenderNode.prototype.query = function(start, end) {
  // Template method: common boilerplate
  var normalized = this.normalizeInput_(start, end);
  if (this.isInvalid_(normalized)) return [];
  
  var results = this.executeQuery_(normalized);
  return this.constructFragments_(results);
};

// Subclasses only implement operator logic:
StretchRenderNode.prototype.executeQuery_ = function(normalized) {
  // Just the stretch logic, ~10 lines
};
```

**Benefit**:
- Reduce JS node definitions by ~200 lines (25 lines × 13 nodes - shared base)
- Make adding new operators faster (just override `executeQuery_()`)
- Consistent error handling across all operators

**Estimate**: 2-3 hours

---

### 2. Complete API Migration

**Context**: Per [docs/standalone-app-restore-plan.md](docs/standalone-app-restore-plan.md) Phase 4

**Current State**:
- New API defined: `nextOnsetTimeFrom()`, `eventsAtTime()` ✓
- Used in engine.js unsynced path ✓
- Used in tests ✓
- Old API (`advance()`, `eventForTime()`) NOT implemented but referenced ✗

**Plan**:
1. Add compatibility aliases (5 min):
   ```javascript
   RenderingTreePlayer.prototype.advance = function(time) {
     return this.nextOnsetTimeFrom(time);
   };
   RenderingTreePlayer.prototype.eventForTime = function(time) {
     var values = this.eventsAtTime(time);
     return values.length > 0 ? {time: time, values: values} : null;
   };
   ```

2. Fix engine.js to use preferred API (5 min)

3. Add migration guide to codebase (10 min)

4. Plan deprecation timeline (optional)

**Estimate**: 20 minutes

---

### 3. Split render-tree.js

**Current**: 280+ lines with 13 factory functions + builder class

**Issue**: Hard to find specific operator implementation

**Proposed Structure**:
```
js/renderer/
├── render-tree.js (80 lines, builder only)
├── nodes/
│   ├── element-render-node.js ✓ (exists)
│   ├── ... (11 more)
│   └── empty-render-node.js ✓ (exists)
├── factories/ (NEW)
│   ├── element-node-factory.js (15 lines)
│   ├── pattern-node-factory.js (60 lines, shared by h/v/t patterns)
│   ├── add-node-factory.js (15 lines)
│   ├── ... etc
│   └── factory-registry.js (10 lines, index of all factories)
└── render-tree-builder.js (50 lines, updated to use factory registry)
```

**Benefit**:
- Easier to locate operator factory logic
- Easier to add new operators
- Better for parallel development

**Estimate**: 1 hour

---

## C++ Analysis

### Positive Findings

**File**: [embedded/renderer/RenderTreePlayer.hpp](embedded/renderer/RenderTreePlayer.hpp)
- ✓ Well-factored, minimal, clean
- ✓ No dead code
- ✓ Private helpers properly scoped
- ✓ Consistent with JS version
- **No refactoring needed**

**File**: [embedded/parser/Parser.cpp](embedded/parser/Parser.cpp)
- ✓ Minimal wrapper (3 lines)
- ✓ Delegates to KrillParser
- ✓ Clean separation of concerns
- **No refactoring needed**

**Test Files**: [embedded/tests/tst_render_tree_player_state_machine.cpp](embedded/tests/tst_render_tree_player_state_machine.cpp), [embedded/tests/tst_run_cases.cpp](embedded/tests/tst_run_cases.cpp)
- ✓ Tests use correct API (`nextOnsetTimeFrom()`, `eventsAtTime()`)
- ✓ Tests are comprehensive
- ✓ No dead code in tests
- ℹ Could extract common test utilities (file-finding pattern, etc.) but not critical

---

## Prioritized Refactoring Roadmap

### Phase 1: Fixes (IMMEDIATE)
**Est. 1.5 hours** — Fix blocker + obvious dead code

1. ✅ Fix engine.js undefined method (5 min)
2. ✅ Remove unused lodash import (1 min)
3. ✅ Add API documentation (5 min)
4. ✅ Consolidate engine event logic (15 min)
5. ✅ Extract time utilities (20 min)
6. ✅ Add fragment validation helper (15 min)

**Result**: Engine works with synced devices, removed dead code, DRY utilities

---

### Phase 2: Extraction (SHORT-TERM)
**Est. 3-4 hours** — Extract duplication patterns

1. Create BaseQueryRenderNode (2-3 hrs)
2. Refactor all 13 render nodes to use base class (1 hr)
3. Complete API migration / add aliases (30 min)

**Result**: 200+ lines removed, cleaner operator implementations, clearer API

---

### Phase 3: Restructuring (MEDIUM-TERM)
**Est. 1-2 hours** — Reorganize for maintainability

1. Split render-tree.js into factory registry pattern (1 hr)
2. Consider breaking engine.js into device-specific modules (optional, 1-2 hrs)

**Result**: Easier to add new operators, clearer code organization

---

## Testing Recommendations

| Test Type | Coverage Gap | Priority |
|-----------|--------------|----------|
| Synced playback | Zero (code is broken) | CRITICAL |
| Each render node | Partial (tests exist but may not cover edge cases) | MEDIUM |
| Edge cases | Some rests/empty patterns tested, but incomplete | MEDIUM |
| Error handling | None - no expected errors tested | LOW |
| API aliases | None (if added) | MEDIUM |

---

## Files Requiring Changes

### Summary Table

| File | Changes | Effort | Priority |
|------|---------|--------|----------|
| [js/playback/engine.js](js/playback/engine.js) | Fix undefined method, consolidate logic | 15 min | 🔴 CRITICAL |
| [js/input-evaluator.js](js/input-evaluator.js) | Remove unused import | 1 min | 🟢 QUICK |
| [js/playback/rendering-tree-player.js](js/playback/rendering-tree-player.js) | Add docs, add aliases (optional) | 10 min | 🟡 MEDIUM |
| [js/utils/time-utils.js](js/utils/time-utils.js) | NEW - extract utilities | 20 min | 🟡 MEDIUM |
| [js/renderer/nodes/query-node-utils.js](js/renderer/nodes/query-node-utils.js) | Add fragment helpers | 15 min | 🟡 MEDIUM |
| [js/renderer/nodes/\*-render-node.js](js/renderer/nodes/) (13 files) | Use base class | 2-3 hrs | 🟡 MEDIUM |
| [js/renderer/render-tree.js](js/renderer/render-tree.js) | Refactor factory pattern | 1 hr | 🟢 OPTIONAL |
| [embedded/\*](embedded/) | No changes needed | — | ✓ |

---

## Summary of Issues Found

### Dead Code (5 items)
1. ❌ **BLOCKER**: `eventForTime()` call in engine.js (runtime error)
2. ❌ Unused lodash import in input-evaluator.js
3. ⚠️ Synced MIDI path in engine.js (either unused or broken)
4. ⚠️ Multiple `queryArc()` methods that could be inlined
5. ℹ️ Test-only paths in playback-device.js (expected, not a problem)

### Duplication (3 major areas)
1. 📋 **Fraction utilities** (~25 lines repeated across 5+ files)
2. 📋 **Query boilerplate** (~200 lines across 13 render nodes)
3. 📋 **Event emission logic** (2 methods in engine.js doing similar things)

### Refactoring Opportunities (3 levels)
1. 🟢 **Quick wins** — Remove dead code, extract utilities (1.5 hours)
2. 🟡 **Medium-term** — Consolidate renderer nodes (2-3 hours)
3. 🟠 **Structural** — Split factories, restructure modules (1-2 hours)

---

## Conclusion

**Recommendation**: Start with Phase 1 (fixes) immediately to prevent runtime crashes and remove dead code. Phase 2 extraction will significantly improve maintainability by reducing duplication by ~300 lines. Phase 3 restructuring is optional but recommended for scalability when adding new operators.

**Expected Outcome After All Phases**:
- ✓ No dead code or broken method calls
- ✓ ~300 lines of duplication removed
- ✓ Cleaner, more maintainable codebase
- ✓ Faster onboarding for new operators
- ✓ Better test coverage

