#include "parser/Parser.hpp"
#include "parser/Helpers.hpp"

#include <third_party/catch2/catch.hpp>
#include <third_party/rapidjson/document.h>
#include <third_party/rapidjson/istreamwrapper.h>

#include <fstream>
#include <string>

using namespace rapidjson;

TEST_CASE("AST parity with shared test-cases-ast.json")
{
  std::ifstream ifs{ R"(../../tests/test-cases-ast.json)" };
  REQUIRE(ifs.is_open());

  IStreamWrapper isw{ ifs };
  Document astDoc;
  REQUIRE(!astDoc.ParseStream(isw).HasParseError());
  REQUIRE(astDoc.IsObject());
  REQUIRE(astDoc.HasMember("cases"));
  REQUIRE(astDoc["cases"].IsObject());

  const auto& cases = astDoc["cases"].GetObject();

  for (const auto& entry : cases)
  {
    REQUIRE(entry.name.IsString());
    const std::string source = entry.name.GetString();
    const auto& expectedAst = entry.value;

    krill::Parser parser;
    Document parsingDocument;
    auto parseResult = parser.parse(parsingDocument, source);
    INFO("source: " << source);
    REQUIRE(parseResult.has_value());

    const bool similar = (parseResult.value() == expectedAst);
    if (!similar)
    {
      INFO("AST mismatch for source: " << source);
      krill::Dump("Expected", expectedAst);
      krill::Dump("Parsed", parseResult.value());
    }
    CHECK(similar);
  }
}
