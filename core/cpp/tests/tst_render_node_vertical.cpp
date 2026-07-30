#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/VerticalPatternRenderNode.hpp"
#include "renderNodeTestUtils.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("VerticalPatternRenderNode query behavior")
{
  using namespace krill;

  std::vector<RenderNodePtr> children;
  children.push_back(std::make_shared<ElementRenderNode>("left"));
  children.push_back(nullptr);
  children.push_back(std::make_shared<ElementRenderNode>("right"));

  VerticalPatternRenderNode node(std::move(children));

  const auto full = node.query({Fraction(0), Fraction(1)});
  REQUIRE(full.size() == 2);
  CHECK(krill::tests::values(full) == std::vector<std::string>{"left", "right"});

  const auto noWidth = node.query({Fraction(1, 2), Fraction(1, 2)});
  CHECK(noWidth.empty());
}
