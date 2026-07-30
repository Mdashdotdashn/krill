#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/ScaleRenderNode.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("ScaleRenderNode query behavior")
{
  using namespace krill;

  std::vector<RenderNodePtr> children;
  children.push_back(std::make_shared<ElementRenderNode>("0"));
  children.push_back(std::make_shared<ElementRenderNode>("1"));
  children.push_back(std::make_shared<ElementRenderNode>("2"));
  children.push_back(std::make_shared<ElementRenderNode>("3"));
  auto source = std::make_shared<HorizontalPatternRenderNode>(std::move(children));

  ScaleRenderNode node("MiNor", source);
  const auto full = node.query({Fraction(0), Fraction(1)});
  REQUIRE(full.size() == 4);
  CHECK(full[0].value == "0");
  CHECK(full[1].value == "2");
  CHECK(full[2].value == "3");
  CHECK(full[3].value == "5");
}
