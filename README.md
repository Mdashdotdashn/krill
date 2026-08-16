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

## data

In krill, there's no assumption of what data can be toyed with. Rather, the 'decoding' of the information is done by the player. In it's current form, there's only one player which is the `GMDevice`. It plays notes on channel 1 and drums on channel 10. To interprete the data, it uses the following rules:
* `~` is a rest and will lead to no output
* If it encouters a number, it transforms it into a midi note based on a C0 (note 36)
* If it encounters a note name (`a` or `a0`), it will play that note
* If it encounters a chord name (`am` or `AMaj` or `c#4m7b5`), it will play the chord in its root position (inversion will come up later) - all chord decoding are handled by tonaljs, see [here](https://github.com/tonaljs/v2/blob/master/packages/dictionary/data/chords.json) for a complete reference
* If it contains a [recognized drum name](https://github.com/Mdashdotdashn/krill/blob/master/js/music/conversion.js#L5), it will play the GM note number equivalent on channel 10

## general principles

krill follow mostly a syntax similar to Tidal Cycle where one first define a pattern and the applies operators to it. For example:

```
slow 2 $ "1 2 3"
```

defines the pattern "1 2 3" and applies the slow operator to it that will slow the speed of the pattern by a factor 2

You can pipe operator one after the other separating them by a `$`:

```
slow 2 $ euclid 5 8 $ "1 2 3"
```

## patterns - basic

the key strength of the system is the way complex patterns can be defined. Patterns are combination of successive element. The most basic pattern is a set of element between quotes:

```
"c0 g1 d#0 b1"
```

By default, a pattern has always a length of one cycle. The meaning of a cycle is purely up to you. You can see it as a bar, a measure or whatever you decide it to be. This means that the more you add steps, the more their respective time will shorten.

`"1 2 3 4"` will play four steps per cycle

`"a b c"` will play thee steps per cycle

This can be very interesting when you combine patterns together. For example, you can use the `,` to define a series of pattern playing at the same time. So something like:

```
"bd sd hh bd, ~ ~ bd"
```

unwraps to:

```
bd .  .  sd .  .  hh .  .  bd .  .
~  .  .  .  ~  .  .  .  bd .  .  .
```

and the second bass drum on a 1/3 signature within a more regular 4 beat phrase. So by default, patterns leads to polyrythmic material.

## patterns - inception

A pattern step can also be a sub-pattern. To say so, you enclose the step between bracket `[]`. For example the following:

```
"1 [a b] 3 4"
```

defines a 4 step pattern with the second step being another pattern. The sub-pattern follows the same rules as defined above, except that instead of having a length of one cycle, it has the length of 1/4th of cycles. So the above pattern expands to something like

```
1 . a b 3 . 4 .
```

Adding a 3rd step in the sub-pattern won't change the playback position of 1,3,4 but will fill the second step with 3 elements.

you can do any level nesting of sub-patterns.

Note that although you don't need to write it, specifying a top-level pattern as `"1 2 3"` is equivalent to writing `"[1 2 3]"`

## patterns - a more traditional approach

Usually, pattern elements are written consequently and one wouldn't expect timing to change as you add more steps. If you want to achieve this, you can opt for forcing the default length of a step within a pattern using the `%` modifier. For example:

```
"[1 2 3]%2"
```

will play the pattern at a speed of 2 steps per cycle adding a 4th step won't change the speed at which steps are played back.

To have steps of various length, you can also use the `@` modifier. For example, if you want to play a syncopated rythm like this one

```
C . E . . C . G"
```

You can contract it by using the following:

```
"[c@2 e@3 c@2 g]"
```

## patterns - modifiers

Modifiers can be viewed as operation applied to patterns or pattern steps. We've already seen a couple of them like `%` or `@`.

Useful step modifiers include:

- `@n` for step weighting
- `%n` for fixed step divisions
- `/n` for stretching
- `*n` for speeding up
- `(steps,pulses)` for Euclidean patterns

Here's a more extensive list of all modifiers:

**@n:** step weight - assign a weight `n` to a step. In some conditions you can also see it as a step length.

`"[c@2 e@3 c@2 g]"`

**%n:**: assigns the step division to be a ratio of a cycle , rather than being computed from the amount of steps inside the pattern

`"[c e c g]%3"`

**/n:**: stretches the pattern by the specified factor. this can also be seen as multiplying the pattern length or slowing it down.
`"[c e c g]/2"`

**\*n:**: contracts the pattern by the specified factor. this can also be seen as dividing the pattern length or speeding it up.
`"[c e c g]/2"`

**(s,p)**: repeat the step according to a bjorklund/euclidian pattern made of `s` steps and `p` pulses

`"bd(5,8)"`

## patterns - inception (deeper level)

In the previous chapter, we've seen we can define patterns as step within a given pattern:
```
[1 [a b] 3 4]
```

with the introduction of modifiers, we can now alter inner pattern in ways that they become longer than one cycle. Two typical examples would be
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

Alternatively, if you change the pattern length to be smaller than one cycle, the pattern content will be repeated until a cycle's worth of data is produced. So for example
```
"bd [hh]*2 sd bd
```
will lead to something equivalent to
```
"bd [hh hh] sd bd
```

This can be used at you advantage to introduce variation of content / rhythm within a pattern.

## operators

Operators transform a pattern using `$`. Operators can be chained from left to right:

```text
slow 2 $ "1 2 3"
slow 2 $ euclid 5 8 $ "bd"
fast 2 $ "bd sd"
struct "t f f t" $ "bd"
euclid 5 8 $ "bd"
```

where the output of the slow operator is the same pattern as the input but stretched by a factor 2 (or slowed down)

Here's a list of the existing operators:

**slow n** slows down or stretches the sequence in time.

`slow 4 $ "c e c g"`

**fast n** speeds up or contracts the sequence in time.

`"hh*8" // build a sequence of 8 hh steps`

**struct p** applies the structure of the boolean pattern p to the content of the input sequence

`struct "t f f t" $ "bd"`  =>  `"bd ~ ~ bd"`

`struct "t f f t" $ "[Cm Em]%1"` => `"[Cm ~ ~ Cm Em ~ ~ Em]%1"`

`struct "t t [t t] t" $ "bd sd"` => `"bd bd [sd sd] sd"`

**scale s** interpretes the numbers from the input sequence as intervals of the scale `s` and outputs the corresponding notes. The scales are handled by tonaljs, see the [list of available scales](https://github.com/tonaljs/v2/blob/master/packages/dictionary/data/scales.json)

`slow 0.25 $ scale "major" $ "0 2 4 6 7 6 4 2"` // Plays the strange days theme

## grouping operators

Grouping operators bundles sequences (including operators) together into one. The only current grouping operator is `cat`

**cat [s1, ..]**: concatenates two or more sequences so that they are played one after the other

`cat [ slow 2 $ "1 12", slow 4 $ "5 17"]`

**stack [s1, ..]**: plays two or more sequence in parallel

`stack [ slow 2 $ "1 12", slow 4 $ "5 17"]`


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

## More Information

- [App documentation](app/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Contributing](docs/CONTRIBUTING.md)
- [Build and test workflow](docs/BUILD-AND-TEST.md)
- [Possible future development](docs/POSSIBLE-FUTURE-DEVELOPMENT.md)
