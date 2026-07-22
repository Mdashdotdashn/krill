#include "renderer/RenderTreeBuilder.hpp"

#include "renderer/RenderTreePlayer.hpp"
#include "parser/Parser.hpp"
#include "testUtils.hpp"

#include <third_party/rapidjson/istreamwrapper.h>

#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <array>

namespace
{
std::ifstream openSharedRunCasesFile()
{
  // Support common CWDs (repo root, embedded/build, embedded/build/tests).
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

std::string expectedValueAsString(const rapidjson::Value& v)
{
  if (v.IsString())
  {
    return v.GetString();
  }

  if (v.IsInt())
  {
    return std::to_string(v.GetInt());
  }

  if (v.IsInt64())
  {
    return std::to_string(v.GetInt64());
  }

  if (v.IsUint())
  {
    return std::to_string(v.GetUint());
  }

  if (v.IsUint64())
  {
    return std::to_string(v.GetUint64());
  }

  if (v.IsDouble())
  {
    std::ostringstream ss;
    ss << v.GetDouble();
    return ss.str();
  }

  return "";
}
} // namespace

TEST_CASE("Rendertree")
{
  using namespace rapidjson;

  // Load shared JS test cases from repository root across common CWDs.
  std::ifstream ifs = openSharedRunCasesFile();
  REQUIRE(ifs.is_open());

  IStreamWrapper isw{ ifs };
  Document document{};
  REQUIRE(!document.ParseStream(isw).HasParseError());
  REQUIRE(document.HasMember("cases"));

  const auto& cases = document["cases"];
  REQUIRE(cases.IsObject());

  for (auto it = cases.MemberBegin(); it != cases.MemberEnd(); ++it)
  {
    REQUIRE(it->name.IsString());
    REQUIRE(it->value.IsObject());

    const auto source = it->name.GetString();
    const auto& expected = it->value;

    std::cout << source << std::endl;

    krill::Parser parser;
    Document parseDoc;
    auto parseResult = parser.parse(parseDoc, source);
    REQUIRE(parseResult.has_value());

    const auto pRenderTree = RenderTreeBuilder::fromJson(parseResult.value());

    RenderTreePlayer player;
    player.setTree(pRenderTree);

    Fraction currentTime(-.001);

    // Loop over the test's expected values.
    for (const auto& m : expected.GetObject())
    {
      std::optional<Cycle::Event> oEvent;
      int guard = 0;
      int noProgressGuard = 0;
      auto lastTime = currentTime;
      while (!(oEvent))
      {
        INFO("No event produced while evaluating source: " << source);
        INFO("Current expected time: " << m.name.GetString());
        REQUIRE(guard++ < 64);
        const auto nextTime = player.advance(currentTime);
        oEvent = player.eventForTime(nextTime);

        if (nextTime == lastTime)
        {
          noProgressGuard++;
          INFO("No-progress advance time: " << nextTime.convertFractionToDouble());
          REQUIRE(noProgressGuard < 64);
        }
        else
        {
          noProgressGuard = 0;
        }

        lastTime = nextTime;
        currentTime = nextTime;
      }

      const auto expectedTimeAsString = m.name.GetString();
      const auto expectedValues = m.value.GetArray();

      currentTime.reduce();
      const auto currentTimeAsString = std::string(currentTime);
      const auto values = oEvent->values;

      CHECK(currentTimeAsString == expectedTimeAsString);
      CHECK(expectedValues.Size() == values.size());

      size_t index = 0;
      for (const auto& v : expectedValues)
      {
        const auto expectedValue = expectedValueAsString(v);
        CHECK(expectedValue == values[index++]);
      }
    }
  }
}