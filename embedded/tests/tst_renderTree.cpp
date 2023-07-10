#include "renderer/RenderTreeBuilder.hpp"
#include "utils/jsonUtils.hpp"
#include "testUtils.hpp"

#include <rapidjson/istreamwrapper.h>

#include <cassert>
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
    const auto use = optionOrValue(v,"use", false);
    const auto expected = v["expected"].GetObject();
    const auto model = v["model"].GetObject();

    if (use)
    {
      const auto pRenderTree = RenderTreeBuilder::fromJson(model);
      pRenderTree->tick();
      const auto cycle = pRenderTree->render();
    }
  }
}