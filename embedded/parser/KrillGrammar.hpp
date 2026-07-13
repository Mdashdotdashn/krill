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

)";
