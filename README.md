Krill
=====
Krill is a NodeJS implementation of a [Tidal Cycle](tidalcycles.org) style live coding interpreter. The main motivation was to keep Tidal's wonderful flexibility while allowing an easiest way to hack it because it's javascript based, not Haskell.

It is under development and therefore constantly in flux and misses tons of features.

Right now Krill only sequences note event through midi, there is no equivalent to superdirt (although there's some plan in the future to allow things along that line)

## installation
Installing krill is pretty much your standard nodejs install:
* clone this repository
* run `npm install` in the cloned repository

You can then run krill using `node main`. This will start a web server listening on port 3000. You can then connect to it using a web browser at `localhost:3000`

A note on midi configuration: there is no form of auto-detection of midi interfaces nor any command line argument for it. You need to edit the file in `js/application.js` and modify the line

>   this.playbackDevice_ = new GMDevice('Microsoft GS Wavetable Synth 0');

Replacing the string by the interface you'd like to use. Annoyingly this is case sensitive and you need to fully type the interface name. Krill will bail out if it can't open the specified interface, but will list the detected interface too so you can copy paste.

## usage

Everything is done by typing in the edit window of a browser pointed at `localhost:3000`. You need to type a sequence (see later for a description of the supported syntax but you can try `"2 3 4"` - including the quotes) followed by Shift-Enter. Sequences are written over paragraphs. If the paragraph contains syntaxic errors, a little sign will be displayed in the gutter of the editor.

To stop the playback, send a sequence containing a single rest, like `"~"`.

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

Usually, pattern elements are written consequently and one wouldn't expect timing to change as you add more steps. If you want to achieve this, you can opt for forcing the default length of a step within a sequence using the `%` modifier. For example:

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
