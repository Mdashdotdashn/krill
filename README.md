Krill
=====
Krill is a livecoding environment inspired from [TidalCycles](https://tidalcycles.org).

The aim of this project is preserve Tidal's wonderful flexibility while allowing an faster way to hack at it in Javascript. At this moment, Krill only sequences note-events via midi, there is no equivalent to Tida's superdirt(although there's some plan in the future to allow things along that line).

It is presently under active development.

## Installation

Installing krill is pretty much your standard nodejs install:

```
git clone https://github.com/Mdashdotdashn/krill.git
cd krill
npm install
node main
```

This will start a web server listening on port 3000, you can then connect to it using a web browser at `localhost:3000`.

A note on midi configuration: there is no form of auto-detection of midi interfaces nor any command line argument for it. You need to edit the file in `js/application.js` and modify the line

>   this.playbackDevice_ = new GMDevice('Microsoft GS Wavetable Synth 0');

Replacing the string by the interface you'd like to use. Annoyingly this is case sensitive and you need to fully type the interface name. Krill will bail out if it can't open the specified interface, but will list the detected interface too so you can copy paste.

## Usage

Everything is done by typing in the edit window of a browser pointed at `localhost:3000`. You need to type a pattern (see later for a description of the supported syntax but you can try `"2 3 4"` - including the quotes) followed by Shift-Enter. Patterns are written over paragraphs. If the paragraph contains syntaxic errors, a little sign will be displayed in the gutter of the editor.

To stop the playback, send a pattern containing a single rest, like `"~"`.

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

Modifiers can be viewed as operation applied to patterns or pattern steps. We've already seen a couple of them like `%` or `@`. Here's a more extensive list of all modifiers:

**@n:** step weight - assign a weight `n` to a step. In some conditions you can also see it as a step length.

`"[c@2 e@3 c@2 g]"`

**%n:**: assigns the step division to be a ratio of a cycle , rather than being computed from the amount of steps inside the pattern

`"[c e c g]%3"`

**/n:**: stretches the pattern by the specified factor. this can also be seen as multiplying the pattern length or slowing it down.
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
"bd [hh]/0.5 sd bd
```
will lead to something equivalent to
```
"bd [hh hh] sd bd
```

This can be used at you advantage to introduce variation of content / rhythm within a pattern.

## operators

Operators are applied to a pattern and result to another pattern. A typical example would be
```
slow 2 $ "1 2 3"
```
where the output of the slow operator is the same pattern as the input but stretched by a factor 2 (or slowed down)

Here's a list of the existing operators:

**slow n** slows down or stretches - assign a weight `n` to a step. In some conditions you can also see it as a step length.

`"[c@2 e@3 c@2 g]"`

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
