# Krill Architecture

## Overview

Krill is a live-coding music notation system with a **dual-implementation strategy**: both C++ and JavaScript implement the same parsing, AST transformation, and rendering pipeline to ensure platform flexibility and performance optimization.

The project is structured into two main components:

- **`/core/`** - Playback infrastructure (both C++ and JS implementations)
- **`/app/`** - Web application frontend (Hapi server + live editor)

## Core Infrastructure (`/core/`)

The core contains the complete playback pipeline implemented in parallel languages:

### Shared Components
- **`test-cases.json`** - Shared test fixtures for pattern evaluation
- **`test-cases-ast.json`** - Shared AST snapshots for parity validation
- **`grammar.txt`** - PEG grammar (used by both implementations)

### C++ Implementation (`/core/cpp/`)

**Purpose**: High-performance parser and renderer for real-time MIDI playback

**Components**:
- **`src/parser/`** - PEG-based parser using cpp-peglib
  - `KrillParser.hpp` - Generated parser from grammar.txt
  - `Parser.hpp` - Parser wrapper with error handling
  - `Context.hpp` - Parsing context and state management
  - `Types.hpp` - AST node definitions

- **`src/renderer/`** - AST-to-render-tree transformation
  - `RenderTreeBuilder.hpp` - Main orchestrator for rendering
  - `factories/` - Operator-specific node factories
  - `nodes/` - Render node implementations for all operators
  - `RenderNode.hpp` - Base class for all render nodes

- **`src/harmony/`** - Music theory utilities
  - `theory/Scale.hpp` - Scale operations
  - `theory/Roman.hpp` - Roman numeral analysis
  - `core/NoteMidi.hpp` - MIDI note conversion
  - `core/Interval.hpp` - Interval calculations

**Build System**: CMake with MinGW64 toolchain
```bash
cd core/cpp
bash prepare_build.sh
cd build
cmake --build .
```

**Tests**: Catch2 framework in `tests/` directory
- Parser tests: `tst_parser.cpp`
- AST parity: `tst_ast_cases.cpp`
- Render contracts: `tst_run_cases.cpp`
- Operator nodes: `tst_render_node_*.cpp`
- Music theory: `tst_harmony_*.cpp`

### JavaScript Implementation (`/core/js/`)

**Purpose**: Node.js runtime implementation for scripting and web environments

**Components**:
- **`input-evaluator.js`** - Main entry point
  - Reads `grammar.txt` and generates PEGjs parser
  - Wraps parser with error handling
  - Exports `Evaluator` class

- **`application.js`** - Hapi application factory
  - Initializes web server with routes
  - Configures MIDI device detection
  - Provides `/grammar.txt` and `/command` endpoints

- **`type.js`** - AST node type definitions
  - Mirrors C++ `Types.hpp`
  - Type checking and validation

- **`patterns/`** - Pattern evaluation
  - `pattern.js` - Core pattern implementation
  - `pattern-event.js` - Event scheduling
  - `weaving.js` - Timeline weaving

- **`renderer/`** - AST-to-render-tree transformation
  - `factory.js` - Node factory dispatcher
  - `operator-nodes/` - Operator implementations
  - `render-node.js` - Base render node class

- **`playback/`** - Audio and MIDI playback
  - `engine.js` - Main playback orchestrator
  - `playback-device.js` - Audio device management
  - `rendering-tree-player.js` - Render tree execution
  - `sync-device.js` - Synchronization utilities

- **`music/`** - Music theory utilities
  - `harmony.js` - Harmony and scale operations
  - `conversion.js` - MIDI/note conversion

**Tests**: Node.js test files with assertion library
```bash
npm test                    # Run all JS tests
npm run update-ast-cases    # Regenerate AST snapshots
```

## Data Flow

The pipeline transforms text notation into scheduled MIDI events:

```
Input Text
    ↓
Parser (PEG grammar)
    ↓
Abstract Syntax Tree (AST)
    ↓
Renderer (Operator evaluation)
    ↓
Render Tree (Operator nodes with state)
    ↓
Player (Timeline execution)
    ↓
MIDI Events
```

### Example
```
Input:    "add(scale(notes(C D E), 0.25), 2)"
AST:      AddNode { patterns: [ScaleNode{...}, 2] }
Render:   Renders patterns in series with fade-in/out
Output:   MIDI note-on/note-off events on timeline
```

## Operator Sets

Both implementations support identical operator sets organized by category:

**Time Operators**:
- `add()` - Sequential composition
- `horizontal()` - Parallel horizontal arrangement
- `vertical()` - Parallel vertical stacking

**Pattern Operators**:
- `pattern()` - Named pattern definition
- `weave()` - Timeline interleaving

**Transformations**:
- `scale()` - Proportional time scaling
- `shift()` - Time offset
- `stretch()` - Non-proportional time stretching
- `trunc()` - Duration truncation

**Structural**:
- `struct()` - Structural containment
- `element()` - Atomic element (notes, rests)
- `bjorklund()` - Euclidean rhythm generation

## Parity Strategy

**Goal**: Ensure C++ and JavaScript produce identical AST and render trees for identical input

**Development Rule**: Unless a change is explicitly platform-specific, feature work in Krill is only considered complete when both the JavaScript and C++ implementations are updated and validated. Partial single-language progress should be tracked as incomplete.

**Validation Method**:
1. Load shared `test-cases.json` and `test-cases-ast.json`
2. Parse each case in both implementations
3. Compare AST structures (must be identical)
4. Compare rendered output (must be identical)
5. Snapshot comparison catches unintended divergence

**Running Parity Tests**:
```bash
npm run test-parity-contract-all
```

This command:
- Runs JS tests
- Rebuilds C++ (if needed)
- Runs C++ tests
- Validates AST snapshots match

Exit code 0 = Complete parity.

## Test Structure

### Shared Fixtures
- **`test-cases.json`** - 100+ pattern evaluation test cases
- **`test-cases-ast.json`** - 50+ AST validation snapshots

### JavaScript Tests (`core/js/tests/`)
- **`test-runner.js`** - Main test aggregator
- **`test-evaluator.js`** - Input parsing and evaluation
- **`test-ast-cases.js`** - AST parity validation
- **`test-parser-canonicalization.js`** - Grammar canonicalization
- **`test-run-cases.js`** - Full pattern execution
- **`test-harmony.js`** - Music theory validation
- **`test-render-nodes.js`** - Operator node verification
- **`test-render-query-contract.js`** - Renderer contracts
- **`test-player-state-machine.js`** - Playback state validation

### C++ Tests (`core/cpp/tests/`)
- **`test.cpp`** - Test framework setup
- **`tst_parser.cpp`** - Parser validation
- **`tst_ast_cases.cpp`** - AST parity (loaded from shared JSON)
- **`tst_run_cases.cpp`** - Pattern evaluation (loaded from shared JSON)
- **`tst_render_node_*.cpp`** - Individual operator nodes
- **`tst_harmony_*.cpp`** - Music theory modules
- **`tst_peglib_*.cpp`** - Grammar smoke tests

## Web Application (`/app/`)

**Purpose**: Live-coding interface for real-time music notation

**Entry Point**: `app/main.js` (Hapi server)

**Endpoints**:
- `GET /` - Main editor interface
- `GET /{file*}` - Static file serving (CSS, JS, HTML)
- `GET /grammar.txt` - Grammar endpoint (for parser updates)
- `GET /command?input=...` - Execute pattern and return AST
- `GET /reporter` - Live MIDI device reporter

**Configuration**:
- Loads core JS implementation via `require('../core/js/application.js')`
- Detects available MIDI output devices
- Serves static assets from `public/` directory

## Build and Run

**Quick Start**:
```bash
# Install dependencies
npm install

# Start web server (port 3000)
npm start

# Run all tests
npm test

# Full parity validation
npm run test-parity-contract-all

# Regenerate AST snapshots (when grammar intentionally changes)
npm run update-ast-cases
```

**Development**:
- JavaScript: Edit files in `core/js/`, tests auto-detect changes
- C++: Edit files in `core/cpp/src/`, rebuild with `cmake --build core/cpp/build`
- Both: Grammar changes in `grammar.txt` require AST snapshot regeneration

## Directory Structure

```
krill/
├── core/                          # Playback infrastructure
│   ├── cpp/                       # C++ implementation
│   │   ├── src/
│   │   │   ├── parser/            # PEG parser
│   │   │   ├── renderer/          # AST renderer
│   │   │   └── harmony/           # Music theory
│   │   ├── tests/                 # C++ test suite
│   │   ├── third_party/           # External dependencies
│   │   ├── build/                 # CMake build directory
│   │   └── CMakeLists.txt
│   ├── js/                        # JavaScript implementation
│   │   ├── input-evaluator.js
│   │   ├── application.js
│   │   ├── type.js
│   │   ├── patterns/
│   │   ├── renderer/
│   │   ├── playback/
│   │   ├── music/
│   │   └── tests/                 # JS test suite
│   ├── test-cases.json            # Shared test fixtures
│   ├── test-cases-ast.json        # Shared AST snapshots
│   ├── grammar.txt                # PEG grammar (shared)
│   └── README.md
├── app/                           # Web application
│   ├── main.js                    # Hapi server entry
│   ├── public/
│   │   ├── index-edit.html
│   │   ├── css/
│   │   └── js/                    # Browser JS and libs
│   └── README.md
├── tests/                         # Integration tests
│   ├── test-parity-contract.js
│   ├── test-parity-contract-all.sh
│   └── README.md
├── docs/
│   ├── ARCHITECTURE.md            # This file
│   ├── render-operator-parity.md
│   └── ...
├── package.json                   # npm configuration
└── README.md                      # Top-level overview
```

## Contributing

When modifying the architecture:

1. **Grammar changes** → Update `core/grammar.txt` → Run `npm run update-ast-cases`
2. **New operators** → Implement in both `core/cpp/src/renderer/` and `core/js/renderer/` → Add tests to both suites
3. **Parser changes** → Update parser logic in both implementations → Ensure parity tests pass
4. **Playback changes** → Modify rendering/player pipeline → Validate with `npm run test-parity-contract-all`

See [CONTRIBUTING.md](../CONTRIBUTING.md) for detailed guidelines.
