#include "../third_party/catch2/catch.hpp"
#include <peglib.h>
#include <string>

// Story 2.2 + 2.3 — Mini-notation core + slice modifiers.
// Structure/matching only — no semantic actions yet.
//
// Strategy: a combined grammar of all S2.1+S2.2+S2.3 rules is defined here.
// matchesRule() prepends "root <- RULE\n" so each test can pick its entry point.

// ── Combined test grammar ─────────────────────────────────────────────────────

static const char* kMiniNotationGrammar = R"(
number              <- minus? int frac? exp?
int                 <- zero / (digit1_9 DIGIT*)
frac                <- '.' DIGIT+
exp                 <- e_char (minus / plus)? DIGIT+
e_char              <- [eE]
digit1_9            <- [1-9]
DIGIT               <- [0-9]
zero                <- '0'
minus               <- '-'
plus                <- '+'
ws                  <- [ \n\r\t]*
comma               <- ws ',' ws
quote               <- ["']
step_char           <- [0-9a-zA-Z~\-#.]
step                <- ws step_char+ ws
sub_cycle           <- ws '[' ws stack ws ']' ws
timeline            <- ws '<' ws single_cycle ws '>' ws
slice               <- sub_cycle / timeline / step
slice_modifier      <- slice_weight / slice_bjorklund / slice_slow / slice_fast / slice_fixed_step
slice_weight        <- '@' number
slice_bjorklund     <- '(' ws number ws comma ws number ws ')'
slice_slow          <- '/' number
slice_fast          <- '*' number
slice_fixed_step    <- '%' number
slice_with_modifier <- slice slice_modifier?
single_cycle        <- slice_with_modifier+
stack               <- single_cycle (comma single_cycle)*
sequence            <- ws quote stack quote
)";

// ── Helper ────────────────────────────────────────────────────────────────────

static bool matchesRule(const char* ruleName, const char* input)
{
    std::string g = std::string("root <- ") + ruleName + "\n" + kMiniNotationGrammar;
    peg::parser p(g.c_str());
    REQUIRE(p); // grammar must be valid
    return p.parse(input);
}

// ── sub_cycle ─────────────────────────────────────────────────────────────────

TEST_CASE("S2.2 sub_cycle", "[mininotation]")
{
    REQUIRE(matchesRule("sub_cycle", "[1 2 3]"));
    REQUIRE(matchesRule("sub_cycle", "[ bd sd hh ]"));       // extra spaces
    REQUIRE(matchesRule("sub_cycle", "[1, 2]"));              // comma stack inside
    REQUIRE(matchesRule("sub_cycle", "[1 [2 3]]"));           // nested sub_cycle
    REQUIRE(matchesRule("sub_cycle", "[bd@2 sd]"));           // with modifier

    REQUIRE_FALSE(matchesRule("sub_cycle", "1 2 3"));         // no brackets
    REQUIRE_FALSE(matchesRule("sub_cycle", "[1 2"));          // unclosed
    REQUIRE_FALSE(matchesRule("sub_cycle", "[]"));            // empty — needs at least one step
}

// ── timeline ──────────────────────────────────────────────────────────────────

TEST_CASE("S2.2 timeline", "[mininotation]")
{
    REQUIRE(matchesRule("timeline", "<1 2 3>"));
    REQUIRE(matchesRule("timeline", "< bd sd >"));
    REQUIRE(matchesRule("timeline", "<1 [2 3]>"));            // sub_cycle inside timeline

    REQUIRE_FALSE(matchesRule("timeline", "1 2 3"));          // no angle brackets
    REQUIRE_FALSE(matchesRule("timeline", "<1 2"));           // unclosed
    REQUIRE_FALSE(matchesRule("timeline", "<>"));             // empty
}

// ── slice ─────────────────────────────────────────────────────────────────────

TEST_CASE("S2.2 slice", "[mininotation]")
{
    REQUIRE(matchesRule("slice", "bd"));                      // plain step
    REQUIRE(matchesRule("slice", "3"));
    REQUIRE(matchesRule("slice", "[1 2]"));                   // sub_cycle
    REQUIRE(matchesRule("slice", "<a b>"));                   // timeline

    REQUIRE_FALSE(matchesRule("slice", ""));
    REQUIRE_FALSE(matchesRule("slice", "@"));                 // modifier char alone
}

// ── slice modifiers (S2.3) ────────────────────────────────────────────────────

TEST_CASE("S2.3 slice_weight", "[mininotation]")
{
    REQUIRE(matchesRule("slice_weight", "@2"));
    REQUIRE(matchesRule("slice_weight", "@0.5"));
    REQUIRE_FALSE(matchesRule("slice_weight", "2"));
    REQUIRE_FALSE(matchesRule("slice_weight", "@"));
}

TEST_CASE("S2.3 slice_bjorklund", "[mininotation]")
{
    REQUIRE(matchesRule("slice_bjorklund", "(3,8)"));
    REQUIRE(matchesRule("slice_bjorklund", "( 3 , 8 )"));
    REQUIRE_FALSE(matchesRule("slice_bjorklund", "(3 8)"));   // missing comma
    REQUIRE_FALSE(matchesRule("slice_bjorklund", "3,8"));     // no parens
}

TEST_CASE("S2.3 slice_slow", "[mininotation]")
{
    REQUIRE(matchesRule("slice_slow", "/2"));
    REQUIRE(matchesRule("slice_slow", "/0.5"));
    REQUIRE_FALSE(matchesRule("slice_slow", "2"));
}

TEST_CASE("S2.3 slice_fast", "[mininotation]")
{
    REQUIRE(matchesRule("slice_fast", "*3"));
    REQUIRE_FALSE(matchesRule("slice_fast", "3"));
}

TEST_CASE("S2.3 slice_fixed_step", "[mininotation]")
{
    REQUIRE(matchesRule("slice_fixed_step", "%4"));
    REQUIRE_FALSE(matchesRule("slice_fixed_step", "4"));
}

// ── slice_with_modifier ───────────────────────────────────────────────────────

TEST_CASE("S2.2 slice_with_modifier", "[mininotation]")
{
    REQUIRE(matchesRule("slice_with_modifier", "bd"));         // no modifier
    REQUIRE(matchesRule("slice_with_modifier", "bd@2"));       // weight
    REQUIRE(matchesRule("slice_with_modifier", "bd/2"));       // slow
    REQUIRE(matchesRule("slice_with_modifier", "bd*3"));       // fast
    REQUIRE(matchesRule("slice_with_modifier", "bd%4"));       // fixed step
    REQUIRE(matchesRule("slice_with_modifier", "bd(3,8)"));    // bjorklund
    REQUIRE(matchesRule("slice_with_modifier", "[1 2]/2"));    // sub_cycle + slow
    REQUIRE(matchesRule("slice_with_modifier", "<a b>@3"));    // timeline + weight
}

// ── single_cycle ──────────────────────────────────────────────────────────────

TEST_CASE("S2.2 single_cycle", "[mininotation]")
{
    REQUIRE(matchesRule("single_cycle", "1"));
    REQUIRE(matchesRule("single_cycle", "1 2 3"));
    REQUIRE(matchesRule("single_cycle", "bd sd hh"));
    REQUIRE(matchesRule("single_cycle", "1 [2 3] 4"));         // nested sub_cycle
    REQUIRE(matchesRule("single_cycle", "1 <2 3> 4"));         // timeline
    REQUIRE(matchesRule("single_cycle", "bd@2 sd"));           // with modifier

    REQUIRE_FALSE(matchesRule("single_cycle", ""));
}

// ── stack ─────────────────────────────────────────────────────────────────────

TEST_CASE("S2.2 stack", "[mininotation]")
{
    REQUIRE(matchesRule("stack", "1 2 3"));                    // single row
    REQUIRE(matchesRule("stack", "1 2, 3 4"));                 // two rows
    REQUIRE(matchesRule("stack", "bd, sd, hh"));               // three rows
    REQUIRE(matchesRule("stack", "1 [2 3], 4 5"));             // sub_cycle in row

    REQUIRE_FALSE(matchesRule("stack", ""));
}

// ── sequence ──────────────────────────────────────────────────────────────────

TEST_CASE("S2.2 sequence", "[mininotation]")
{
    REQUIRE(matchesRule("sequence", "\"1 2 3\""));
    REQUIRE(matchesRule("sequence", "'bd sd hh'"));            // single quotes
    REQUIRE(matchesRule("sequence", "\"1 [2 3] 4\""));         // sub_cycle
    REQUIRE(matchesRule("sequence", "\"1 <2 3> 4\""));         // timeline
    REQUIRE(matchesRule("sequence", "\"1 2, 3 4\""));          // stack (comma)
    REQUIRE(matchesRule("sequence", "\"bd@2 sd\""));           // modifier
    REQUIRE(matchesRule("sequence", "\"c#4 d#0 g1\""));        // note names

    REQUIRE_FALSE(matchesRule("sequence", "1 2 3"));           // no quotes
    REQUIRE_FALSE(matchesRule("sequence", "\"\""));            // empty content
    REQUIRE_FALSE(matchesRule("sequence", "\"1 2 3"));         // unclosed
}
