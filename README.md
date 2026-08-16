# Krill

Krill is a live-coding pattern language for sequencing MIDI notes. It is inspired by [TidalCycles](https://tidalcycles.org) and runs as a browser-based editor backed by a Node.js server.
In parallel, there is a C++ implementation of the engine that is on par with the JavaScript equivalent.

This document describes how to get Krill running and provides a primer for the language.

If you are interested in the development side of things, feel free to have a look at:

- [Krill Node App documentation](app/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Contributing](docs/CONTRIBUTING.md)
- [Build and test workflow](docs/BUILD-AND-TEST.md)
- [Possible future development](docs/POSSIBLE-FUTURE-DEVELOPMENT.md)


## Start Krill

From the repository root:

```bash
npm install
npm start
```

Open [http://localhost:3000](http://localhost:3000) in a browser. Enter a pattern in the editor and press Shift-Enter to play it.

To stop playback, send the command `hush`:

```text
hush
```

## MIDI Setup

By default Krill looks for a loopback MIDI output:

- Windows: loopMIDI
- macOS: IAC
- Linux: MIDI Through

Select a device explicitly when starting the server:

```bash
node app/main.js --midi-device 'loopMidi'
```

You can also select a MIDI sync input:

```bash
node app/main.js --midi-sync 'MIDI input device'
```

The server listens on port 3000 by default. Change it with:

```bash
node app/main.js --port 3001
```

## Data

In Krill, there is no assumption about what data can be used. Rather, the player decodes the information. In its current form, there is only one player, `GMDevice`. It plays notes on channel 1 and drums on channel 10. It interprets the data using the following rules:
* `~` is a rest and produces no output
* If it encounters a number, it transforms it into a MIDI note based on C0 (note 36)
* If it encounters a note name (`a` or `a0`), it will play that note
* If it encounters a chord name (`am`, `AMaj`, or `c#4m7b5`), it plays the chord in root position. Chord decoding is handled by Tonal; see [the chord dictionary](https://github.com/tonaljs/v2/blob/master/packages/dictionary/data/chords.json) for a complete reference.
* If it contains a [recognized drum name](https://github.com/Mdashdotdashn/krill/blob/master/core/js/music/conversion.js#L5), it plays the equivalent GM note number on channel 10.

## General Principles

Krill mostly follows a syntax similar to TidalCycles, where you first define a pattern and then apply operators to it. For example:

```
slow 2 $ '1 2 3'
```

This defines the pattern `'1 2 3'` and applies the `slow` operator, slowing the pattern by a factor of 2.

You can pipe operators one after another, separating them with `$`:

```
slow 2 $ euclid 5 8 $ '1 2 3'
```

## Patterns - Basic

The key strength of the system is the way complex patterns can be defined. Patterns are combinations of successive elements. The most basic pattern is a set of elements between quotes:

```
'c0 g1 d#0 b1'
```

By default, a pattern has always a length of one cycle. The meaning of a cycle is purely up to you. You can see it as a bar, a measure or whatever you decide it to be. This means that the more you add steps, the more their respective time will shorten.

`'1 2 3 4'` will play four steps per cycle

`'a b c'` will play three steps per cycle.

This can be very interesting when you combine patterns together. For example, you can use the `,` to define a series of pattern playing at the same time. So something like:

```
'bd sd hh bd, ~ ~ bd'
```

unwraps to:

```
bd .  .  sd .  .  hh .  .  bd .  .
~  .  .  .  ~  .  .  .  bd .  .  .
```

The second pattern places its bass drum on a third-note subdivision within a more regular four-beat phrase. By default, patterns therefore produce polyrhythmic material.

## Patterns - Inception

A pattern step can also be a sub-pattern. To do so, enclose the step in brackets (`[]`). For example:

```
'1 [a b] 3 4'
```

This defines a four-step pattern with the second step being another pattern. The sub-pattern follows the same rules as above, except that instead of having a length of one cycle, it has a length of one quarter of a cycle. The pattern expands to something like:

```
1 . a b 3 . 4 .
```

Adding a third step to the sub-pattern will not change the playback positions of 1, 3, and 4, but will fill the second step with three elements.

You can nest sub-patterns to any depth.

Note that although you don't need to write it, specifying a top-level pattern as `'1 2 3'` is equivalent to writing `'[1 2 3]'`

## Patterns - A More Traditional Approach

Usually, pattern elements are written consecutively, and you would not expect timing to change as you add more steps. To achieve this, you can force the default length of a step within a pattern using the `%` modifier. For example:

```
'[1 2 3]%2'
```

This plays the pattern at two steps per cycle; adding a fourth step will not change the playback speed.

To have steps of various lengths, you can also use the `@` modifier. For example, to play a syncopated rhythm like this one:

```
'C ~ E ~ ~ C ~ G'
```

You can contract it by using the following:

```
'[c@2 e@3 c@2 g]'
```

## Patterns - Modifiers

Modifiers can be viewed as operations applied to patterns or pattern steps. We have already seen a couple of them, such as `%` and `@`.

Useful step modifiers include:

- `@n` for step weighting
- `%n` for fixed step divisions
- `/n` for stretching
- `*n` for speeding up
- `(steps,pulses)` for Euclidean patterns

Here's a more extensive list of all modifiers:

**@n:** step weight - assign a weight `n` to a step. In some conditions you can also see it as a step length.

`'[c@2 e@3 c@2 g]'`

**%n:** assigns the step division as a ratio of a cycle, rather than computing it from the number of steps in the pattern.

`'[c e c g]%3'`

**/n:** stretches the pattern by the specified factor. This can also be seen as multiplying the pattern length or slowing it down.
`'[c e c g]/2'`

**\*n:** contracts the pattern by the specified factor. This can also be seen as dividing the pattern length or speeding it up.
`'[c e c g]*2'`

**(s,p)**: repeat the step according to a Bjorklund/Euclidean pattern made of `s` steps and `p` pulses.

`'bd(5,8)'`

## Patterns - Inception (Deeper Level)

In the previous section, we saw that patterns can be defined as steps within a given pattern:
```
[1 [a b] 3 4]
```

With modifiers, we can alter inner patterns so that they become longer than one cycle. Two typical examples are:
```
[1 [a b]%1 3 4]
```
or
```
[1 [a b]/2 3 4]
```

(these two are actually equivalent)

In these cases, only one cycle worth of the inner pattern will be used at a time. This means the two examples above lead to the following:

```
[1 a 3 4] followed by [1 b 3 4]
```

Alternatively, if you change the pattern length to be smaller than one cycle, the pattern content will be repeated until one cycle of data is produced. For example:
```
'bd [hh]*2 sd bd
```
will lead to something equivalent to
```
'bd [hh hh] sd bd
```

This can be used to your advantage to introduce variation in content and rhythm within a pattern.

## operators

Operators transform a pattern using `$`. Operators can be chained from left to right:

```text
slow 2 $ '1 2 3'
slow 2 $ euclid 5 8 $ 'bd'
fast 2 $ 'bd sd'
struct 't f f t' $ 'bd'
euclid 5 8 $ 'bd'
```

The output of the `slow` operator is the same pattern as the input, but stretched by a factor of 2 (or slowed down).

Here's a list of the existing operators:

**slow n** slows down or stretches the sequence in time.

`slow 4 $ 'c e c g'`

**fast n** speeds up or contracts the sequence in time.

`'hh*8' // build a sequence of 8 hh steps`

**struct p** applies the structure of the boolean pattern p to the content of the input sequence

`struct 't f f t' $ 'bd'`  =>  `'bd ~ ~ bd'`

`struct 't f f t' $ '[Cm Em]%1'` => `'[Cm ~ ~ Cm Em ~ ~ Em]%1'`

`struct 't t [t t] t' $ 'bd sd'` => `'bd bd [sd sd] sd'`

**scale s** interprets the numbers from the input sequence as intervals of scale `s` and outputs the corresponding notes. Scales are handled by Tonal; see the [list of available scales](https://github.com/tonaljs/v2/blob/master/packages/dictionary/data/scales.json).

`slow 0.25 $ scale 'major' $ '0 2 4 6 7 6 4 2'` // Plays the strange days theme

## Grouping Operators

Grouping operators bundle sequences, including operators, together. The current grouping operators are `cat` and `stack`.

**cat [s1, ..]**: concatenates two or more sequences so that they are played one after the other

`cat [ slow 2 $ '1 12', slow 4 $ '5 17']`

**stack [s1, ..]**: plays two or more sequences in parallel.

`stack [ slow 2 $ '1 12', slow 4 $ '5 17']`


`trunc`, `rotL`, and `rotR` shorten or rotate patterns:

```text
trunc 0.75 $ '1 2 3 4'
rotR 0.125 $ 'bd ~ sd ~'
rotL 0.125 $ 'bd ~ sd ~'
```

## Velocity

Velocity can be attached to an individual event with `:`:

```text
'bd:100 sd'
'bd:60 sd:127 hh:32'
```

A leaf event may use an absolute MIDI velocity from `0` to `127`.

A normalized value from `0` to `1` acts as a multiplier of the default or inherited velocity:

```text
'hh:0.8'
'[bd sd]:0.5'
```

## More Information

- [App documentation](app/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Contributing](docs/CONTRIBUTING.md)
- [Build and test workflow](docs/BUILD-AND-TEST.md)
- [Possible future development](docs/POSSIBLE-FUTURE-DEVELOPMENT.md)
