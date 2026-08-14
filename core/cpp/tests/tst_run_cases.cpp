#include "../src/renderer/RenderTreeBuilder.hpp"
#include "../src/renderer/RenderTreePlayer.hpp"
#include "../src/parser/Parser.hpp"

#include "../third_party/catch2/catch.hpp"
#include "../../third_party/rapidjson/document.h"
#include "../../third_party/rapidjson/istreamwrapper.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace
{
std::ifstream openSharedRunCasesFile()
{
  const std::array<const char*, 7> candidatePaths = {
    "../test-cases.json",
    "../../test-cases.json",
    "../../../test-cases.json",
    "../../../../test-cases.json",
    "../../../../../test-cases.json",
    "../../../../../../test-cases.json",
    "../../../../../../../test-cases.json"
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
  struct ExpectedEntry
  {
    std::string value;
    std::map<std::string, std::string> controls;

    bool operator==(const ExpectedEntry& other) const
    {
      return value == other.value && controls == other.controls;
    }
  };

  std::vector<ExpectedEntry> entries;
};

std::string formatDouble(double value)
{
  if (std::fabs(value - std::round(value)) < 1e-9)
  {
    return std::to_string(static_cast<long>(std::llround(value)));
  }

  std::string text = std::to_string(value);
  while (!text.empty() && text.back() == '0')
  {
    text.pop_back();
  }
  if (!text.empty() && text.back() == '.')
  {
    text.pop_back();
  }
  return text;
}

std::string rapidValueToString(const rapidjson::Value& value)
{
  if (value.IsString())
  {
    return value.GetString();
  }
  if (value.IsBool())
  {
    return value.GetBool() ? "true" : "false";
  }
  if (value.IsInt())
  {
    return std::to_string(value.GetInt());
  }
  if (value.IsInt64())
  {
    return std::to_string(value.GetInt64());
  }
  if (value.IsUint())
  {
    return std::to_string(value.GetUint());
  }
  if (value.IsUint64())
  {
    return std::to_string(value.GetUint64());
  }
  if (value.IsDouble())
  {
    return formatDouble(value.GetDouble());
  }
  return "";
}

ExpectedEvent::ExpectedEntry parseExpectedEntry(const rapidjson::Value& value)
{
  ExpectedEvent::ExpectedEntry parsed;

  if (value.IsString())
  {
    parsed.value = value.GetString();
    return parsed;
  }

  REQUIRE(value.IsObject());
  REQUIRE(value.HasMember("value"));
  parsed.value = rapidValueToString(value["value"]);

  if (value.HasMember("controls"))
  {
    REQUIRE(value["controls"].IsObject());
    for (const auto& control : value["controls"].GetObject())
    {
      REQUIRE(control.name.IsString());
      parsed.controls[control.name.GetString()] = rapidValueToString(control.value);
    }
  }

  return parsed;
}

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
      parsed.entries.push_back(parseExpectedEntry(v));
    }

    events.push_back(std::move(parsed));
  }

  std::sort(events.begin(), events.end(), [](const ExpectedEvent& a, const ExpectedEvent& b) {
    return a.time < b.time;
  });

  return events;
}

std::vector<ExpectedEvent::ExpectedEntry> entriesAtTime(krill::RenderTreePlayer& player, const Fraction& time)
{
  std::vector<ExpectedEvent::ExpectedEntry> entries;

  const auto fragments = player.queryPointWindow(time);
  for (const auto& fragment : fragments)
  {
    if (fragment.wholeStart != time)
    {
      continue;
    }

    ExpectedEvent::ExpectedEntry entry;
    entry.value = fragment.value;
    entry.controls = fragment.controls;
    entries.push_back(std::move(entry));
  }

  return entries;
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
      std::vector<std::string> values;
      int guard = 0;

      while (values.empty())
      {
        nextTime = player.nextOnsetTimeFrom(currentTime);
        values = player.eventsAtTime(nextTime);
        currentTime = nextTime;
        guard += 1;

        if (guard > 4096)
        {
          FAIL("Stuck while advancing player for source");
        }
      }

      CHECK(nextTime == expectedEvent.time);
      CHECK(entriesAtTime(player, nextTime) == expectedEvent.entries);
    }
  }
}
