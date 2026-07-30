#include "../third_party/catch2/catch.hpp"
#include <peglib.h>
#include "parser/KrillGrammar.hpp"

// Story 2.6 — Commands (hush, setcps, setbpm) and the top-level start/statement rule.
// This is the first test file that uses kKrillGrammar directly as the full grammar,
// since start is now the first rule.

// ── Helper ────────────────────────────────────────────────────────────────────

static peg::parser& getParser()
{
    static peg::parser p(kKrillGrammar);
    return p;
}

static bool parse(const char* input)
{
    auto& p = getParser();
    REQUIRE(p); // grammar must be valid
    return p.parse(input);
}

// ── grammar validity ──────────────────────────────────────────────────────────

TEST_CASE("S2.6 kKrillGrammar compiles", "[statement]")
{
    peg::parser p(kKrillGrammar);
    REQUIRE(p);
}

// ── hush ──────────────────────────────────────────────────────────────────────

TEST_CASE("S2.6 hush command", "[statement]")
{
    REQUIRE(parse("hush"));
    REQUIRE(parse("  hush  "));     // surrounding whitespace

    REQUIRE_FALSE(parse("hushing")); // not a prefix match for step (hushing is a step)
    // Note: "hushing" parses as a sequence_definition (step), not a hush command
    // so we just verify hush itself is unambiguous
}

// ── setcps ────────────────────────────────────────────────────────────────────

TEST_CASE("S2.6 setcps command", "[statement]")
{
    REQUIRE(parse("setcps 0.5"));
    REQUIRE(parse("setcps 1"));
    REQUIRE(parse("setcps 2.0"));

    REQUIRE_FALSE(parse("setcps"));         // missing value
    REQUIRE_FALSE(parse("setcps abc"));     // non-numeric value
}

// ── setbpm ────────────────────────────────────────────────────────────────────

TEST_CASE("S2.6 setbpm command", "[statement]")
{
    REQUIRE(parse("setbpm 120"));
    REQUIRE(parse("setbpm 140.5"));

    REQUIRE_FALSE(parse("setbpm"));
}

// ── sequence statements ───────────────────────────────────────────────────────

TEST_CASE("S2.6 statement — sequences", "[statement]")
{
    REQUIRE(parse("\"1 2 3\""));
    REQUIRE(parse("'bd sd hh'"));
    REQUIRE(parse("\"c#4 d#0 g1\""));
    REQUIRE(parse("\"1 [2 3] <4 5>\""));
    REQUIRE(parse("\"1 2, 3 4\""));
}

TEST_CASE("S2.6 statement — operator chains", "[statement]")
{
    REQUIRE(parse("slow 2 $ \"1 2 3\""));
    REQUIRE(parse("fast 2 $ euclid 5 8 $ \"bd\""));
    REQUIRE(parse("rotR 1 $ \"1 2 3 4\""));
    REQUIRE(parse("scale \"Cmaj\" $ \"1 2 3\""));
    REQUIRE(parse("slow 2 $ euclid 5 8 $ \"1 [2 3] <4 5>\""));
}

TEST_CASE("S2.6 statement — with trailing comment", "[statement]")
{
    REQUIRE(parse("\"1 2 3\" // my pattern"));
    REQUIRE(parse("slow 2 $ \"1 2 3\" // half speed"));
}

TEST_CASE("S2.6 statement — grouping operators", "[statement]")
{
    REQUIRE(parse("cat [\"1 2\", \"3 4\"]"));
    REQUIRE(parse("stack [\"bd sd\", \"hh\"]"));
    REQUIRE(parse("slow 2 $ cat [\"1 2\", \"3 4\"]"));
}
