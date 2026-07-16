#include "renderer/RenderTreeBuilder.hpp"

#include "renderer/RenderTreePlayer.hpp"
#include "parser/Parser.hpp"
#include "testUtils.hpp"

#include <third_party/rapidjson/istreamwrapper.h>

#include <cassert>
#include <iostream>
#include <fstream>

TEST_CASE("Rendertree")
{
  using namespace rapidjson;

  // Load shared JS test cases from repository root.
  std::ifstream ifs{ R"(../../../tests/test-cases.json)" };
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
      while (!(oEvent))
      {
        const auto nextTime = player.advance(currentTime);
        oEvent = player.eventForTime(nextTime);
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
        CHECK(v.GetString() == values[index++]);
      }
    }
  }
}