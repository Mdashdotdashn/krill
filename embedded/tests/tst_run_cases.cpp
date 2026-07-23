#include "renderer/RenderTreeBuilder.hpp"
#include "renderer/RenderTreePlayer.hpp"
#include "parser/Parser.hpp"

#include <third_party/catch2/catch.hpp>
#include <third_party/rapidjson/document.h>
#include <third_party/rapidjson/istreamwrapper.h>

#include <array>
#include <fstream>
#include <string>

namespace
{
std::ifstream openSharedRunCasesFile()
{
  const std::array<const char*, 6> candidatePaths = {
    "tests/test-cases.json",
    "../tests/test-cases.json",
    "../../tests/test-cases.json",
    "../../../tests/test-cases.json",
    "../../../../tests/test-cases.json",
    "../../../../../tests/test-cases.json"
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

std::vector<std::string> valuesAtTime(const std::vector<krill::QueryFragment>& fragments, const Fraction& expectedTime)
{
  std::vector<std::string> values;
  for (const auto& fragment : fragments)
  {
    if (fragment.wholeStart == expectedTime)
    {
      values.push_back(fragment.value);
    }
  }
  return values;
}
} // namespace

TEST_CASE("Rendertree")
{
  using namespace rapidjson;

  std::ifstream ifs = openSharedRunCasesFile();
  REQUIRE(ifs.is_open());

  IStreamWrapper isw{ifs};
  Document document{};
  REQUIRE(!document.ParseStream(isw).HasParseError());
  REQUIRE(document.HasMember("cases"));
  REQUIRE(document["cases"].IsObject());

  const auto& cases = document["cases"].GetObject();

  for (const auto& entry : cases)
  {
    REQUIRE(entry.name.IsString());
    REQUIRE(entry.value.IsObject());

    const std::string source = entry.name.GetString();
    const auto& expected = entry.value.GetObject();

    krill::Parser parser;
    Document parseDoc;
    auto parseResult = parser.parse(parseDoc, source);
    INFO("source: " << source);
    REQUIRE(parseResult.has_value());

    auto pTree = krill::RenderTreeBuilder::fromJson(parseResult.value());
    krill::RenderTreePlayer player;
    player.setTree(pTree);

    for (const auto& expectedEntry : expected)
    {
      REQUIRE(expectedEntry.name.IsString());
      REQUIRE(expectedEntry.value.IsArray());

      const auto expectedTime = Fraction(std::string(expectedEntry.name.GetString()));
      const auto fragments = player.queryPointWindow(expectedTime);
      const auto actualValues = valuesAtTime(fragments, expectedTime);

      std::vector<std::string> expectedValues;
      for (const auto& v : expectedEntry.value.GetArray())
      {
        REQUIRE(v.IsString());
        expectedValues.push_back(v.GetString());
      }

      CHECK(actualValues == expectedValues);
    }
  }
}
