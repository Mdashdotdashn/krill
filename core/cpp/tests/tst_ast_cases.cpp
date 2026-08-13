#include "../src/parser/Parser.hpp"
#include "../src/parser/Helpers.hpp"

#include "../third_party/catch2/catch.hpp"
#include "../../third_party/rapidjson/document.h"
#include "../../third_party/rapidjson/istreamwrapper.h"

#include <array>
#include <fstream>
#include <string>

using namespace rapidjson;

namespace
{
std::ifstream openSharedAstCasesFile()
{
  const std::array<const char*, 7> candidatePaths = {
    "../test-cases-ast.json",
    "../../test-cases-ast.json",
    "../../../test-cases-ast.json",
    "../../../../test-cases-ast.json",
    "../../../../../test-cases-ast.json",
    "../../../../../../test-cases-ast.json",
    "../../../../../../../test-cases-ast.json"
  };

  for (const auto* path : candidatePaths)
  {
    std::ifstream ifs(path);
    if (ifs.is_open())
    {
      return ifs;
    }
  }

  return std::ifstream{};
}
} // namespace

TEST_CASE("AST parity with shared test-cases-ast.json")
{
  std::ifstream ifs = openSharedAstCasesFile();
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
