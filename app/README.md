# Web App

Node.js/Hapi server and browser-based editor for live coding Krill patterns. Uses the JavaScript implementation from `core/js/` for pattern evaluation and MIDI playback.

## Quick Start

```bash
npm install
npm start
```

Then visit `http://localhost:3000` in your browser.

## Files

- **main.js** — Hapi server entry point
  - Serves static assets from `public/`
  - Routes HTTP requests to pattern parser and MIDI output
- **application.js** — Bridge between server and core/js modules
- **public/index-edit.html** — Editor UI
- **public/js/** — Client-side editor code (Ace, jQuery terminal, d3)
- **public/css/** — Stylesheets

## Server Routes

- `GET /` — Serves editor UI
- `GET /{file*}` — Static file serving (public folder)
- `GET /grammar.txt` — Pattern grammar reference
- `GET /command?command=<pattern>` — Parse and evaluate a pattern
- `GET /reporter` — Drain reported events

## Configuration

The server runs on port 3000 by default. To change:

```bash
node main.js --port 3001
```

MIDI device selection:

```bash
node main.js --midi-device "loopMidi"
node main.js --midi-sync "MIDI input device"
node main.js --cycle "slow 2 $ ..."
```

For a list of available devices, check the JavaScript MIDI library (easymidi) output.

## Development

To run with live reloading during development:

```bash
npm install -g nodemon
nodemon app/main.js
```

The editor UI code is in `public/` and will hot-reload on browser refresh.

## Architecture

```
main.js (Hapi server)
  ↓
app.parse(input)  [from ../core/js/application.js]
  ↓
Evaluator.evaluate() → Renderer → RenderTree → Player → MIDI
```

The `application.js` module:
1. Parses input into an AST using the JavaScript evaluator
2. Builds a render tree from the AST
3. Creates a player that sequences MIDI events per cycle
4. Sends MIDI to the connected device

## Dependencies

See `package.json` at root for full dependency list. Key packages:

- `@hapi/hapi` — Web framework
- `easymidi` — MIDI I/O
- `pegjs` — Parser generator (used by core/js)
- `tonal` — Music theory library
- Client: Ace editor, jQuery terminal, d3
