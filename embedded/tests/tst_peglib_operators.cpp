#include <third_party/catch2/catch.hpp>
#include <peglib.h>
#include <string>

// Story 2.4 + 2.5 — Operators, grouping, and sequence_or_operator.
// Bundled because operator rules (add, struct) and sequence_or_operator
// are mutually recursive and cannot compile separately.
// Structure/matching only — no semantic actions yet.

// ── Combined test grammar (S2.1 through S2.5) ─────────────────────────────────

static const char* kOperatorsGrammar = R"(
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
operator_argument   <- step / sequence_or_operator
operator            <- add / scale / slow / fast / bjorklund / struct / rotR / rotL
add                 <- 'add' ws operator_argument
struct              <- 'struct' ws sequence_or_operator
bjorklund           <- 'euclid' ws int ws int
slow                <- 'slow' ws number
fast                <- 'fast' ws number
rotL                <- 'rotL' ws operator_argument
rotR                <- 'rotR' ws operator_argument
scale               <- 'scale' ws quote step_char+ quote
comment             <- '//' [^\n]*
cat                 <- 'cat' ws '[' ws sequence_or_operator (comma sequence_or_operator)* ws ']'
stack_op            <- 'stack' ws '[' ws sequence_or_operator (comma sequence_or_operator)* ws ']'
group_operator      <- cat / stack_op
sequence_or_group   <- group_operator / sequence
sequence_or_operator <- sequence_or_group ws comment*
                      / operator ws '$' ws sequence_or_operator
sequence_definition <- sequence_or_operator / comment
)";

static bool matchesRule(const char* ruleName, const char* input)
{
    std::string g = std::string("root <- ") + ruleName + "\n" + kOperatorsGrammar;
    peg::parser p(g.c_str());
    REQUIRE(p);
    return p.parse(input);
}

// ── comment ───────────────────────────────────────────────────────────────────

TEST_CASE("S2.4 comment", "[operators]")
{
    REQUIRE(matchesRule("comment", "// hello"));
    REQUIRE(matchesRule("comment", "//"));               // empty comment
    REQUIRE(matchesRule("comment", "// slow 2 $ \"1\"")); // comment looks like code

    REQUIRE_FALSE(matchesRule("comment", "# not a comment"));
    REQUIRE_FALSE(matchesRule("comment", "1 2 3"));
}

// ── individual operators ──────────────────────────────────────────────────────

TEST_CASE("S2.4 slow / fast", "[operators]")
{
    REQUIRE(matchesRule("slow", "slow 2"));
    REQUIRE(matchesRule("slow", "slow 0.5"));
    REQUIRE(matchesRule("fast", "fast 3"));
    REQUIRE(matchesRule("fast", "fast 2.5"));

    REQUIRE_FALSE(matchesRule("slow", "slow"));          // missing number
    REQUIRE_FALSE(matchesRule("slow", "2"));             // missing keyword
}

TEST_CASE("S2.4 bjorklund (euclid)", "[operators]")
{
    REQUIRE(matchesRule("bjorklund", "euclid 5 8"));
    REQUIRE(matchesRule("bjorklund", "euclid 3 16"));

    REQUIRE_FALSE(matchesRule("bjorklund", "euclid 5"));   // missing second int
    REQUIRE_FALSE(matchesRule("bjorklund", "euclid"));
}

TEST_CASE("S2.4 rotL / rotR", "[operators]")
{
    REQUIRE(matchesRule("rotL", "rotL 1"));
    REQUIRE(matchesRule("rotR", "rotR 0.5"));
    REQUIRE(matchesRule("rotL", "rotL \"0 0.125\""));
    REQUIRE(matchesRule("rotR", "rotR '<0 0.125>'"));

    REQUIRE_FALSE(matchesRule("rotL", "rotL"));
}

TEST_CASE("S2.4 scale", "[operators]")
{
    REQUIRE(matchesRule("scale", "scale \"Cmaj\""));
    REQUIRE(matchesRule("scale", "scale 'minor'"));

    REQUIRE_FALSE(matchesRule("scale", "scale Cmaj"));    // unquoted
    REQUIRE_FALSE(matchesRule("scale", "scale"));
}

TEST_CASE("S2.4 add", "[operators]")
{
    REQUIRE(matchesRule("add", "add c0"));                // step argument
    REQUIRE(matchesRule("add", "add \"c0 e0\""));         // sequence argument

    REQUIRE_FALSE(matchesRule("add", "add"));
}

TEST_CASE("S2.4 struct", "[operators]")
{
    REQUIRE(matchesRule("struct", "struct \"1 0 1 0\""));

    REQUIRE_FALSE(matchesRule("struct", "struct"));
}

// ── grouping (S2.5) ───────────────────────────────────────────────────────────

TEST_CASE("S2.5 cat", "[operators]")
{
    REQUIRE(matchesRule("cat", "cat [\"1 2\", \"3 4\"]"));
    REQUIRE(matchesRule("cat", "cat [\"bd\", \"sd\", \"hh\"]"));  // three args

    REQUIRE_FALSE(matchesRule("cat", "cat []"));           // empty
    REQUIRE_FALSE(matchesRule("cat", "cat \"1 2\""));      // no brackets
}

TEST_CASE("S2.5 stack_op", "[operators]")
{
    REQUIRE(matchesRule("stack_op", "stack [\"1 2\", \"3 4\"]"));

    REQUIRE_FALSE(matchesRule("stack_op", "stack []"));
}

// ── sequence_or_operator (the main recursive rule) ───────────────────────────

TEST_CASE("S2.5 sequence_or_operator — bare sequence", "[operators]")
{
    REQUIRE(matchesRule("sequence_or_operator", "\"1 2 3\""));
    REQUIRE(matchesRule("sequence_or_operator", "'bd sd hh'"));
    REQUIRE(matchesRule("sequence_or_operator", "\"1 2 3\" // comment"));  // trailing comment
}

TEST_CASE("S2.5 sequence_or_operator — single operator", "[operators]")
{
    REQUIRE(matchesRule("sequence_or_operator", "slow 2 $ \"1 2 3\""));
    REQUIRE(matchesRule("sequence_or_operator", "fast 2 $ \"bd sd\""));
    REQUIRE(matchesRule("sequence_or_operator", "euclid 5 8 $ \"bd\""));
    REQUIRE(matchesRule("sequence_or_operator", "rotR 1 $ \"1 2 3\""));
    REQUIRE(matchesRule("sequence_or_operator", "scale \"Cmaj\" $ \"1 2 3\""));
    REQUIRE(matchesRule("sequence_or_operator", "struct \"1 0 1 0\" $ \"bd sd\""));
}

TEST_CASE("S2.5 sequence_or_operator — chained operators", "[operators]")
{
    REQUIRE(matchesRule("sequence_or_operator", "slow 2 $ euclid 5 8 $ \"bd sd\""));
    REQUIRE(matchesRule("sequence_or_operator", "fast 2 $ slow 3 $ rotR 1 $ \"1 2 3\""));
}

TEST_CASE("S2.5 sequence_or_operator — grouping operators as source", "[operators]")
{
    REQUIRE(matchesRule("sequence_or_operator", "cat [\"1 2\", \"3 4\"]"));
    REQUIRE(matchesRule("sequence_or_operator", "slow 2 $ cat [\"1 2\", \"3 4\"]"));
    REQUIRE(matchesRule("sequence_or_operator", "stack [\"bd\", \"sd\"]"));
}
