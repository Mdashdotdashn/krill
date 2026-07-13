#include <third_party/catch2/catch.hpp>
#include <peglib.h>

#include "parser/Context.hpp"
#include "parser/KrillGrammar.hpp"
#include "parser/KrillParser.hpp"
#include "parser/XmlBuilder.hpp"
#include "parser/Helpers.hpp"

#include <memory>
#include <string>

// Epic 3 — Semantic actions tests.
// Each story adds test cases to this file as actions are wired.
//
// Strategy: each story tests its actions using a focused grammar (the rule
// under test as root) with the user-data pattern that matches KrillParser.
// This isolates each action without requiring the full pipeline to be complete.

// ── Shared utilities (mirror KrillParser.cpp internals) ───────────────────────

using PValue = std::shared_ptr<rapidjson::Value>;

struct UserData { krill::Context& ctx; };

static std::string trimWs(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    const auto last  = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

// Run a focused grammar with the given rule as root.
// Returns the PValue produced by the rule's action (captured via a root wrapper).
static PValue parseWithAction(
    const std::string& ruleName,
    const std::string& grammar,
    const std::function<void(peg::parser&)>& wireActions,
    const std::string& input,
    rapidjson::Document& doc)
{
    std::string fullGrammar = "root <- " + ruleName + "\n" + grammar;
    peg::parser p(fullGrammar.c_str());
    REQUIRE(p);

    wireActions(p);

    PValue result;
    p["root"] = [&result](const peg::SemanticValues& vs) {
        if (!vs.empty()) result = std::any_cast<PValue>(vs[0]);
    };

    krill::Context ctx(doc, input);
    UserData ud{ctx};
    std::any dt = ud;
    p.parse(input.c_str(), dt);
    return result;
}

// Serialise a rapidjson::Value to a JSON string for easy comparison in assertions.
static std::string toJson(const rapidjson::Value& v)
{
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> w(buf);
    v.Accept(w);
    return buf.GetString();
}

// ── S3.1 grammar fragment ─────────────────────────────────────────────────────

static const char* kStepGrammar = R"(
    step      <- ws step_char+ ws
    step_char <- [0-9a-zA-Z~\-#.]
    ws        <- [ \t\n\r]*
)";

static void wireStepAction(peg::parser& p)
{
    p["step"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
        auto& ud = std::any_cast<UserData&>(dt);
        const auto token = trimWs(vs.token_to_string());
        return std::make_shared<rapidjson::Value>(krill::buildXmlForElement(ud.ctx, token));
    };
}

// ── S3.1 tests ────────────────────────────────────────────────────────────────

TEST_CASE("S3.1 step action — plain step produces element", "[actions]")
{
    rapidjson::Document doc;

    auto checkStep = [&](const char* input, const char* expectedSource) {
        auto result = parseWithAction("step", kStepGrammar, wireStepAction, input, doc);
        REQUIRE(result != nullptr);
        REQUIRE(result->IsObject());
        REQUIRE(std::string((*result)["type_"].GetString()) == "element");
        REQUIRE(std::string((*result)["source_"].GetString()) == expectedSource);
    };

    checkStep("bd",     "bd");
    checkStep("  bd  ", "bd");      // leading/trailing whitespace stripped
    checkStep("~",      "~");       // rest marker
    checkStep("c#4",    "c#4");     // note name with accidental
    checkStep("d#0",    "d#0");
    checkStep("hh",     "hh");
    checkStep("3",      "3");       // numeric step
}

TEST_CASE("S3.1 step action — output JSON shape", "[actions]")
{
    rapidjson::Document doc;
    auto result = parseWithAction("step", kStepGrammar, wireStepAction, "a", doc);

    REQUIRE(result != nullptr);
    // Must have exactly type_ and source_ keys
    REQUIRE(result->MemberCount() == 2);
    REQUIRE(result->HasMember("type_"));
    REQUIRE(result->HasMember("source_"));
    // Matches JS side: {"type_":"element","source_":"a"}
    REQUIRE(toJson(*result) == R"({"type_":"element","source_":"a"})");
}

// ── S3.2 / S3.3 / S3.4 tests — full mini-notation via KrillParser ────────────
//
// KrillParser::parse() now returns results for sequence inputs.
// Test cases mirror the existing tst_parser.cpp "slice" section exactly.

namespace {

// Helper: parse a quoted sequence string and return the JSON result.
// e.g. checkSeq("a")  parses  "\"a\""  as a Krill statement.
std::string parseSeq(krill::KrillParser& p, const std::string& inner)
{
    rapidjson::Document doc;
    const auto input = "\"" + inner + "\"";
    auto result = p.parse(doc, input);
    REQUIRE(result.has_value());
    return toJson(result.value());
}

} // namespace

TEST_CASE("S3.2-S3.4 single step", "[actions]")
{
    krill::KrillParser p;
    // Single step → element (matches tst_parser.cpp "slice" section)
    REQUIRE(parseSeq(p, "a") == R"({"type_":"element","source_":"a"})");
}

TEST_CASE("S3.2-S3.4 horizontal sequence", "[actions]")
{
    krill::KrillParser p;
    REQUIRE(parseSeq(p, "a b") ==
        R"({"type_":"pattern","arguments_":{"alignment":"h"},"source_":[)"
        R"({"type_":"element","source_":"a"},)"
        R"({"type_":"element","source_":"b"}]})");
}

TEST_CASE("S3.2-S3.4 vertical stack", "[actions]")
{
    krill::KrillParser p;
    // "a b, c" → vertical of [horizontal [a,b], c]
    REQUIRE(parseSeq(p, "a b, c") ==
        R"({"type_":"pattern","arguments_":{"alignment":"v"},"source_":[)"
        R"({"type_":"pattern","arguments_":{"alignment":"h"},"source_":[)"
        R"({"type_":"element","source_":"a"},{"type_":"element","source_":"b"}]},)"
        R"({"type_":"element","source_":"c"}]})");
}

TEST_CASE("S3.2-S3.4 sub_cycle", "[actions]")
{
    krill::KrillParser p;
    // "a [2,4]" → horizontal of [a, vertical [2,4]]
    REQUIRE(parseSeq(p, "a [2,4]") ==
        R"({"type_":"pattern","arguments_":{"alignment":"h"},"source_":[)"
        R"({"type_":"element","source_":"a"},)"
        R"({"type_":"pattern","arguments_":{"alignment":"v"},"source_":[)"
        R"({"type_":"element","source_":"2"},{"type_":"element","source_":"4"}]}]})");
}

TEST_CASE("S3.3 timeline changes alignment to t", "[actions]")
{
    krill::KrillParser p;
    // "<a b>" inside a sequence → pattern with alignment "t"
    REQUIRE(parseSeq(p, "<a b>") ==
        R"({"type_":"pattern","arguments_":{"alignment":"t"},"source_":[)"
        R"({"type_":"element","source_":"a"},{"type_":"element","source_":"b"}]})");
}

TEST_CASE("S3.3 timeline single element passes through", "[actions]")
{
    krill::KrillParser p;
    // Single-step timeline → just the element
    REQUIRE(parseSeq(p, "<a>") == R"({"type_":"element","source_":"a"})");
}

TEST_CASE("S3.2-S3.4 nested sub_cycle", "[actions]")
{
    krill::KrillParser p;
    // "[bd sd]" as the whole sequence → horizontal pattern
    REQUIRE(parseSeq(p, "[bd sd]") ==
        R"({"type_":"pattern","arguments_":{"alignment":"h"},"source_":[)"
        R"({"type_":"element","source_":"bd"},{"type_":"element","source_":"sd"}]})");
}

// ── S3.5 tests — operators ────────────────────────────────────────────────────

namespace {
// Parse a full Krill statement (no outer quotes — used for operator expressions).
std::string parseStmt(krill::KrillParser& p, const std::string& input)
{
    rapidjson::Document doc;
    auto result = p.parse(doc, input);
    REQUIRE(result.has_value());
    return toJson(result.value());
}
} // namespace

TEST_CASE("S3.5 slow operator", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(slow 2 $ "a b")");
    REQUIRE(json.find(R"("type_":"stretch")") != std::string::npos);
    const bool hasArgs = json.find(R"("arguments_":[2.0])") != std::string::npos ||
                         json.find(R"("arguments_":[2])")   != std::string::npos;
    REQUIRE(hasArgs);
    REQUIRE(json.find(R"("type_":"pattern")") != std::string::npos);
}

TEST_CASE("S3.5 fast operator", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(fast 2 $ "a")");
    REQUIRE(json.find(R"("type_":"stretch")") != std::string::npos);
    // fast 2 → stretch 0.5
    REQUIRE(json.find("0.5") != std::string::npos);
}

TEST_CASE("S3.5 euclid operator", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(euclid 5 8 $ "bd")");
    REQUIRE(json.find(R"("type_":"bjorklund")") != std::string::npos);
    REQUIRE(json.find("5") != std::string::npos);
    REQUIRE(json.find("8") != std::string::npos);
}

TEST_CASE("S3.5 rotR operator", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(rotR 1 $ "a b c")");
    REQUIRE(json.find(R"("type_":"shift")") != std::string::npos);
}

TEST_CASE("S3.5 scale operator", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(scale "Cmaj" $ "1 2 3")");
    REQUIRE(json.find(R"("type_":"scale")") != std::string::npos);
    REQUIRE(json.find("Cmaj") != std::string::npos);
}

TEST_CASE("S3.5 chained operators", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(slow 2 $ euclid 5 8 $ "bd sd")");
    // Outer node is stretch, inner is bjorklund
    REQUIRE(json.find(R"("type_":"stretch")") != std::string::npos);
    REQUIRE(json.find(R"("type_":"bjorklund")") != std::string::npos);
}

TEST_CASE("S3.5 cat grouping operator", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, R"(cat ["a b", "c d"])");
    REQUIRE(json.find(R"("alignment":"t")") != std::string::npos);
}

// ── S3.6 tests — commands ─────────────────────────────────────────────────────

TEST_CASE("S3.6 hush command", "[actions]")
{
    krill::KrillParser p;
    REQUIRE(parseStmt(p, "hush") == R"({"type_":"command","name_":"hush"})");
}

TEST_CASE("S3.6 setcps command", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, "setcps 0.5");
    REQUIRE(json.find(R"("type_":"command")") != std::string::npos);
    REQUIRE(json.find(R"("name_":"setcps")") != std::string::npos);
    REQUIRE(json.find("0.5") != std::string::npos);
}

TEST_CASE("S3.6 setbpm command", "[actions]")
{
    krill::KrillParser p;
    const auto json = parseStmt(p, "setbpm 120");
    // setbpm 120 → setcps (120/120/2) = 0.5
    REQUIRE(json.find(R"("name_":"setcps")") != std::string::npos);
    REQUIRE(json.find("0.5") != std::string::npos);
}

