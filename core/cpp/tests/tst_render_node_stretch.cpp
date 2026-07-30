#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/StretchRenderNode.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("StretchRenderNode query behavior")
{
  using namespace krill;

  std::vector<RenderNodePtr> children;
  children.push_back(std::make_shared<ElementRenderNode>("1"));
  children.push_back(std::make_shared<ElementRenderNode>("2"));
  children.push_back(std::make_shared<ElementRenderNode>("3"));

  auto source = std::make_shared<HorizontalPatternRenderNode>(std::move(children));
  StretchRenderNode node(source, Fraction(2));

  const auto at0 = node.query({Fraction(0), Fraction(1, 1024)});
  REQUIRE(at0.size() == 1);
  CHECK(at0[0].value == "1");
  CHECK(at0[0].wholeStart == Fraction(0));
  CHECK(at0[0].wholeEnd == Fraction(2, 3));

  const auto atTwoThirds = node.query({Fraction(2, 3), Fraction(2, 3) + Fraction(1, 1024)});
  REQUIRE(atTwoThirds.size() == 1);
  CHECK(atTwoThirds[0].value == "2");
  CHECK(atTwoThirds[0].wholeStart == Fraction(2, 3));
  CHECK(atTwoThirds[0].wholeEnd == Fraction(4, 3));
}
