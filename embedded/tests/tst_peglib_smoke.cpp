#include <third_party/catch2/catch.hpp>
#include <peglib.h>

// Smoke test: verify cpp-peglib is reachable and functional in this build.
// Parses a trivial grammar to confirm the library compiles and runs correctly.

TEST_CASE("cpp-peglib smoke test", "[peglib]")
{
    peg::parser parser(R"(
        root    <- ws token ws
        token   <- [a-zA-Z0-9]+
        ws      <- [ \t]*
    )");

    REQUIRE(parser);  // grammar compiled without errors

    std::string captured;
    parser["token"] = [&](const peg::SemanticValues& vs) {
        captured = vs.token_to_string();
    };

    REQUIRE(parser.parse("  hello  "));
    REQUIRE(captured == "hello");

    REQUIRE_FALSE(parser.parse(""));      // empty input should fail
    REQUIRE_FALSE(parser.parse("!@#"));   // no token chars should fail
}
