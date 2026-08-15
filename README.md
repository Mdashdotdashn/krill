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

A quoted sequence plays one cycle. A cycle can be thought of as a bar, measure, or any other repeating time unit:

```text
"c0 g1 d#0 b1"
"bd sd hh bd"
```

Adding more steps divides that cycle into more parts. For example, these patterns play four and three events per cycle:

```text
"1 2 3 4"
"a b c"
```

Numbers are MIDI-relative note values based on C0. Note names, chords, and recognized drum names are also supported. `~` is a rest.

Patterns are evaluated cyclically, so they can be layered and combined to create polyrhythms.

Patterns can be nested:

```text
"1 [a b] 3 4"
```

Use commas for simultaneous patterns:

```text
"bd sd hh bd, ~ ~ bd"
```

The comma creates parallel material with its own timing. The example above places a four-step drum pattern alongside a three-step pattern.

## Nested Patterns

A step can contain another pattern. The nested pattern fills the time occupied by that step:

```text
"1 [a b] 3 4"
```

The outer pattern has four steps. The second step contains `a b`, so the inner pattern is played within that step while the outer `1`, `3`, and `4` retain their positions.

Nested patterns can be combined with modifiers. A nested pattern longer than one cycle is consumed one cycle at a time:

```text
"[1 [a b]%1 3 4]"
"[1 [a b]/2 3 4]"
```

These produce successive variations equivalent to `[1 a 3 4]` followed by `[1 b 3 4]`.

Conversely, a shorter nested pattern repeats to fill its step:

```text
"bd [hh]*2 sd bd"
```

This is equivalent to `"bd [hh hh] sd bd"` for its timing.

## Operators

Operators transform a pattern using `$`. Operators can be chained from left to right:

```text
slow 2 $ "1 2 3"
slow 2 $ euclid 5 8 $ "bd"
fast 2 $ "bd sd"
struct "t f f t" $ "bd"
euclid 5 8 $ "bd"
```

### Timing Modifiers

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

`@n` changes the relative weight of a step, which changes its duration within a sequence:

```text
"[c@2 e@3 c@2 g]"
```

`%n` forces a pattern to occupy a fixed number of steps per cycle:

```text
"[c e c g]%3"
```

`/n` stretches a pattern, while `*n` contracts it:

```text
"[c e c g]/2"
"[c e c g]*2"
```

`(steps,pulses)` repeats a step according to an Euclidean rhythm:

```text
"bd(5,8)"
```

### Pattern Operators

`slow n` stretches a pattern by a factor:

```text
slow 4 $ "c e c g"
```

`fast n` contracts a pattern. For example, `"hh*8"` creates eight hi-hat steps in a cycle.

`struct pattern` applies a boolean structure to another pattern:

```text
struct "t f f t" $ "bd"
```

This produces `bd ~ ~ bd`.

`scale name` interprets numeric values as degrees of a scale:

```text
scale "major" $ "0 2 4 6 7 6 4 2"
```

`euclid steps pulses` creates Euclidean rhythmic placement:

```text
euclid 5 8 $ "bd"
```

`cat` concatenates patterns sequentially:

```text
cat [ slow 2 $ "1 12", slow 4 $ "5 17" ]
```

`add` transposes note values:

```text
add 2 $ "c3 e3 g3"
```

`trunc`, `rotL`, and `rotR` shorten or rotate patterns:

```text
trunc 0.75 $ "1 2 3 4"
rotR 0.125 $ "bd ~ sd ~"
rotL 0.125 $ "bd ~ sd ~"
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
