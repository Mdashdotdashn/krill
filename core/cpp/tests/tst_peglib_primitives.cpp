#include "../third_party/catch2/catch.hpp"
#include <peglib.h>

// Story 2.1 — Primitives: number, step_char, step, ws, comma, quote
//
// Each section uses a self-contained grammar with the rule-under-test first,
// since cpp-peglib starts parsing from the first defined rule.
// No semantic actions yet — Epic 2 is structure/matching only.

// ── Helpers ───────────────────────────────────────────────────────────────────

static bool matches(const char* grammar, const char* input)
{
    peg::parser p(grammar);
    REQUIRE(p); // grammar itself must be valid
    return p.parse(input);
}

// ── number ────────────────────────────────────────────────────────────────────

static const char* kNumberGrammar = R"(
    number     <- minus? int frac? exp?
    int        <- zero / (digit1_9 DIGIT*)
    frac       <- '.' DIGIT+
    exp        <- e_char (minus / plus)? DIGIT+
    e_char     <- [eE]
    digit1_9   <- [1-9]
    DIGIT      <- [0-9]
    zero       <- '0'
    minus      <- '-'
    plus       <- '+'
)";

TEST_CASE("S2.1 number", "[primitives]")
{
    REQUIRE(matches(kNumberGrammar, "0"));
    REQUIRE(matches(kNumberGrammar, "42"));
    REQUIRE(matches(kNumberGrammar, "-3"));
    REQUIRE(matches(kNumberGrammar, "3.14"));
    REQUIRE(matches(kNumberGrammar, "-0.5"));
    REQUIRE(matches(kNumberGrammar, "1e10"));
    REQUIRE(matches(kNumberGrammar, "2.5e-3"));
    REQUIRE(matches(kNumberGrammar, "1E+2"));

    REQUIRE_FALSE(matches(kNumberGrammar, ""));
    REQUIRE_FALSE(matches(kNumberGrammar, "abc"));
    REQUIRE_FALSE(matches(kNumberGrammar, ".5"));  // frac without leading int
}

// ── ws ────────────────────────────────────────────────────────────────────────

static const char* kWsGrammar = R"(
    ws  <- [ \n\r\t]*
)";

TEST_CASE("S2.1 ws", "[primitives]")
{
    REQUIRE(matches(kWsGrammar, ""));          // zero whitespace is valid
    REQUIRE(matches(kWsGrammar, " "));
    REQUIRE(matches(kWsGrammar, "   "));
    REQUIRE(matches(kWsGrammar, "\t\n\r"));
    REQUIRE_FALSE(matches(kWsGrammar, "a"));
}

// ── quote ─────────────────────────────────────────────────────────────────────

static const char* kQuoteGrammar = R"(
    quote  <- ["']
)";

TEST_CASE("S2.1 quote", "[primitives]")
{
    REQUIRE(matches(kQuoteGrammar, "\""));
    REQUIRE(matches(kQuoteGrammar, "'"));

    REQUIRE_FALSE(matches(kQuoteGrammar, "a"));
    REQUIRE_FALSE(matches(kQuoteGrammar, ""));
}

// ── comma ─────────────────────────────────────────────────────────────────────

static const char* kCommaGrammar = R"(
    comma  <- ws ',' ws
    ws     <- [ \n\r\t]*
)";

TEST_CASE("S2.1 comma", "[primitives]")
{
    REQUIRE(matches(kCommaGrammar, ","));
    REQUIRE(matches(kCommaGrammar, " , "));
    REQUIRE(matches(kCommaGrammar, "\t,\n"));

    REQUIRE_FALSE(matches(kCommaGrammar, ""));
    REQUIRE_FALSE(matches(kCommaGrammar, "a"));
}

// ── step_char ─────────────────────────────────────────────────────────────────

static const char* kStepCharGrammar = R"(
    step_char  <- [0-9a-zA-Z~\-#.]
)";

TEST_CASE("S2.1 step_char", "[primitives]")
{
    REQUIRE(matches(kStepCharGrammar, "a"));
    REQUIRE(matches(kStepCharGrammar, "Z"));
    REQUIRE(matches(kStepCharGrammar, "9"));
    REQUIRE(matches(kStepCharGrammar, "~"));
    REQUIRE(matches(kStepCharGrammar, "-"));
    REQUIRE(matches(kStepCharGrammar, "#"));
    REQUIRE(matches(kStepCharGrammar, "."));

    REQUIRE_FALSE(matches(kStepCharGrammar, ""));
    REQUIRE_FALSE(matches(kStepCharGrammar, " "));
    REQUIRE_FALSE(matches(kStepCharGrammar, "@"));
    REQUIRE_FALSE(matches(kStepCharGrammar, "\""));
}

// ── step ──────────────────────────────────────────────────────────────────────

static const char* kStepGrammar = R"(
    step       <- ws step_char+ ws
    step_char  <- [0-9a-zA-Z~\-#.]
    ws         <- [ \n\r\t]*
)";

TEST_CASE("S2.1 step", "[primitives]")
{
    REQUIRE(matches(kStepGrammar, "bd"));
    REQUIRE(matches(kStepGrammar, "  bd  "));     // surrounding whitespace ok
    REQUIRE(matches(kStepGrammar, "c#4"));
    REQUIRE(matches(kStepGrammar, "~"));           // rest
    REQUIRE(matches(kStepGrammar, "a0"));
    REQUIRE(matches(kStepGrammar, "3"));
    REQUIRE(matches(kStepGrammar, "d#0"));

    REQUIRE_FALSE(matches(kStepGrammar, ""));
    REQUIRE_FALSE(matches(kStepGrammar, "   "));  // whitespace only, no step_chars
    REQUIRE_FALSE(matches(kStepGrammar, "@bd"));  // @ is not a step_char
}
