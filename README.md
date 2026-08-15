# Krill

Krill is a live-coding pattern language for sequencing MIDI notes. It is inspired by [TidalCycles](https://tidalcycles.org) and runs as a browser-based editor backed by a Node.js server.

## Start Krill

From the repository root:

```bash
npm install
npm start
```

Open [http://localhost:3000](http://localhost:3000) in a browser. Enter a pattern in the editor and press Shift-Enter to play it.

To stop playback, send a single rest:

```text
"~"
```

## MIDI Setup

By default Krill looks for a loopback MIDI output:

- Windows: loopMIDI
- macOS: IAC
- Linux: MIDI Through

Select a device explicitly when starting the server:

```bash
node app/main.js --midi-device "loopMidi"
```

You can also select a MIDI sync input:

```bash
node app/main.js --midi-sync "MIDI input device"
```

The server listens on port 3000 by default. Change it with:

```bash
node app/main.js --port 3001
```

## Patterns

A quoted sequence plays one cycle:

```text
"c0 g1 d#0 b1"
"bd sd hh bd"
```

Numbers are MIDI-relative note values based on C0. Note names, chords, and recognized drum names are also supported. `~` is a rest.

Patterns can be nested:

```text
"1 [a b] 3 4"
```

Use commas for simultaneous patterns:

```text
"bd sd hh bd, ~ ~ bd"
```

## Operators

Operators transform a pattern using `$`:

```text
slow 2 $ "1 2 3"
fast 2 $ "bd sd"
struct "t f f t" $ "bd"
euclid 5 8 $ "bd"
```

Useful step modifiers include:

- `@n` for step weighting
- `%n` for fixed step divisions
- `/n` for stretching
- `*n` for speeding up
- `(steps,pulses)` for Euclidean patterns

Examples:

```text
"[c@2 e@3 c@2 g]"
"[c e c g]%3"
"bd(5,8)"
```

## Velocity

Velocity can be attached to an individual event with `:`:

```text
"bd:100 sd"
"bd:60 sd:127 hh:32"
```

A leaf event may use an absolute MIDI velocity from `0` to `127`.

A normalized value from `0` to `1` acts as a multiplier of the default or inherited velocity:

```text
"hh:0.8"
"[bd sd]:0.5"
```

Grouped and nested patterns use normalized velocity factors. Final MIDI velocity is resolved separately for each emitted event.

## More Information

- [App documentation](app/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Contributing](docs/CONTRIBUTING.md)
- [Build and test workflow](docs/BUILD-AND-TEST.md)
- [Possible future development](docs/POSSIBLE-FUTURE-DEVELOPMENT.md)
