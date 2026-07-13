#pragma once

// Krill PEG grammar for cpp-peglib.
// Incrementally built across Epic 2 stories — each section is tagged [S2.x].
// Mirrors grammar.txt (the pegjs grammar used by the JS side).
//
// cpp-peglib differences from pegjs:
//   - Rules use  <-  instead of  =
//   - Comments use  #
//   - Semantic actions are C++ lambdas attached separately, not inline
//   - Character class syntax is identical

static constexpr auto kKrillGrammar = R"(

# ── [S2.1] Numbers ────────────────────────────────────────────────────────────

number      <- minus? int frac? exp?
decimal_pt  <- '.'
digit1_9    <- [1-9]
DIGIT       <- [0-9]
e_char      <- [eE]
exp         <- e_char (minus / plus)? DIGIT+
frac        <- decimal_pt DIGIT+
int         <- zero / (digit1_9 DIGIT*)
minus       <- '-'
plus        <- '+'
zero        <- '0'

# ── [S2.1] Delimiters ─────────────────────────────────────────────────────────

ws          <- [ \n\r\t]*
comma       <- ws ',' ws
quote       <- [\"']

# ── [S2.1] Steps ──────────────────────────────────────────────────────────────

step_char   <- [0-9a-zA-Z~\-#.]
step        <- ws step_char+ ws

# ── [S2.2] Mini-notation core ─────────────────────────────────────────────────

# sub_cycle: bracketed stack  e.g. [1 2, 3 [4]]
sub_cycle           <- ws '[' ws stack ws ']' ws

# timeline: angle-bracket single_cycle  e.g. <1 3 [3 5]>
timeline            <- ws '<' ws single_cycle ws '>' ws

# slice: one atom — step, sub_cycle, or timeline
slice               <- sub_cycle / timeline / step

# ── [S2.3] Slice modifiers ────────────────────────────────────────────────────
# (included here because slice_with_modifier depends on them)

slice_modifier      <- slice_weight / slice_bjorklund / slice_slow / slice_fast / slice_fixed_step
slice_weight        <- '@' number
slice_bjorklund     <- '(' ws number ws comma ws number ws ')'
slice_slow          <- '/' number
slice_fast          <- '*' number
slice_fixed_step    <- '%' number

# ── [S2.2 cont.] ──────────────────────────────────────────────────────────────

# slice with optional modifier  e.g. bd@4  or  [1 2]/2
slice_with_modifier <- slice slice_modifier?

# single_cycle: one or more successive slice_with_modifier (horizontal)
single_cycle        <- slice_with_modifier+

# stack: comma-separated single_cycles (vertical)
stack               <- single_cycle (comma single_cycle)*

# sequence: a quoted stack  e.g. "1 2 3"  or  '1 [2 3], 4'
sequence            <- ws quote stack quote

)";
