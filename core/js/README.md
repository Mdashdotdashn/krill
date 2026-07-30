# JavaScript Implementation

The primary runtime for Krill. Parser, renderer, playback engine, and music theory modules for live-coding pattern evaluation and MIDI sequencing.

## Structure

```
js/
├── input-evaluator.js       # Main parser + evaluator (PEGjs)
├── renderer/                # Render tree nodes, factories, query contract
├── playback/                # Playback engine, device I/O, state machine
├── music/                   # Harmony, scale, note conversion
├── patterns/                # Pattern utilities
├── utils/                   # General utilities
├── type.js                  # Type definitions
├── tests/                   # Unit tests
└── README.md
```

## Key Modules

### Parser & Evaluator
- **input-evaluator.js** — Uses PEGjs to parse text into AST; `Evaluator` class

### Renderer
- **renderer/render-tree.js** — Render node class hierarchy (mirroring C++ operators)
- **renderer/factories/** — Node factory classes
- **renderer/query-contract.js** — Query execution engine

### Playback
- **playback/rendering-tree-player.js** — Stateful tree player (cycles through evaluation)
- **playback/engine.js** — Scheduling and timing
- **playback/playback-device.js** — MIDI output abstraction
- **playback/sync-device.js** — Sync clock

### Music Theory
- **music/harmony.js** — Harmony utilities
- **music/conversion.js** — Note parsing, drum mapping, chord resolution

## Running Tests

```bash
# From root
npm test

# Or from core/js/
npm test

# Update AST snapshots (after grammar changes)
npm run update-ast-cases
```

## Using in the Web App

The web app (app/main.js) depends on this module:

```javascript
const app = require('../core/js/application.js');
app.init(options);
const result = app.parse(inputString);
```

The `application.js` module bridges the server and playback infrastructure.

## Parity with C++

All operator implementations are kept in parity with the C++ version. When adding or changing operators:

1. Update both `core/js/renderer/` and `core/cpp/src/renderer/`
2. Ensure both produce identical render nodes for the same input
3. Run parity tests: `npm run test-parity-contract-all` (from root)
4. Update [../../docs/render-operator-parity.md](../../docs/render-operator-parity.md) if adding new operators

## Development Workflow

1. **Make changes** to parsing logic or operators
2. **Run tests** — `npm test` (or focused test files)
3. **Check parity** — `npm run test-parity-contract-all`
4. **Update snapshots if needed** — `npm run update-ast-cases`
5. **Commit** with description of changes
