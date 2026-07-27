#include "renderer/RenderTreeBuilder.hpp"
#include "renderer/RenderTreePlayer.hpp"
#include "parser/Parser.hpp"

#include <third_party/catch2/catch.hpp>
#include <third_party/rapidjson/document.h>
#include <third_party/rapidjson/istreamwrapper.h>

#include <array>
#include <algorithm>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

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

struct ExpectedEvent
{
  Fraction time{0};
  std::vector<std::string> values;
};

std::vector<ExpectedEvent> sortedExpectedEvents(const rapidjson::Value::ConstObject& expected)
{
  std::vector<ExpectedEvent> events;

  for (const auto& expectedEntry : expected)
  {
    REQUIRE(expectedEntry.name.IsString());
    REQUIRE(expectedEntry.value.IsArray());

    ExpectedEvent parsed;
    parsed.time = Fraction(std::string(expectedEntry.name.GetString()));

    for (const auto& v : expectedEntry.value.GetArray())
    {
      REQUIRE(v.IsString());
      parsed.values.push_back(v.GetString());
    }

    events.push_back(std::move(parsed));
  }

  std::sort(events.begin(), events.end(), [](const ExpectedEvent& a, const ExpectedEvent& b) {
    return a.time < b.time;
  });

  return events;
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
    player.reset();

    const auto expectedEvents = sortedExpectedEvents(expected);
    Fraction currentTime(-1, 10000);

    for (const auto& expectedEvent : expectedEvents)
    {
      Fraction nextTime;
      std::optional<krill::RenderTreePlayer::Event> event;
      int guard = 0;

      while (!event)
      {
        nextTime = player.advance(currentTime);
        event = player.eventForTime(nextTime);
        currentTime = nextTime;
        guard += 1;

        if (guard > 4096)
        {
          FAIL("Stuck while advancing player for source");
        }
      }

      CHECK(nextTime == expectedEvent.time);
      CHECK(event->values == expectedEvent.values);
    }
  }
}
