#include "renderer/RenderTreeBuilder.hpp"

#include "renderer/RenderTreePlayer.hpp"
#include "utils/jsonUtils.hpp"
#include "testUtils.hpp"

#include <third_party/rapidjson/istreamwrapper.h>

#include <cassert>
#include <iostream>
#include <fstream>

TEST_CASE("Rendertree")
{
  using namespace rapidjson;

  // Load the json document will all cases

  std::ifstream ifs{ R"(../../tests/test_cases.json)" };
  assert(ifs.is_open());

  IStreamWrapper isw{ ifs };
  Document document{};
  assert(!document.ParseStream(isw).HasParseError());

  const auto& cases = document["cases"];
  assert(cases.IsArray());

  for (auto& v : cases.GetArray())
  {
    const auto source = v["source"].GetString();
    const auto use = optionOrValue(v, "use", false);
    const auto expected = v["expected"].GetObject();
    const auto model = v["model"].GetObject();

    auto runTest = use;
    // If you want to run a single test, set the string here
    // runTest = (!strcmp(source, "'[a]*4'"));
    if (runTest)
    {
      std::cout << source << std::endl;
      const auto pRenderTree = RenderTreeBuilder::fromJson(model);
      RenderTreePlayer player;
      player.setTree(pRenderTree);

      Fraction currentTime(-.001);

      // Loop over the test's expected values
      for (const auto& m : expected)
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
    else
    {
      std::cout << ".. skip (" << source << ")" << std::endl;
    }
  }
}