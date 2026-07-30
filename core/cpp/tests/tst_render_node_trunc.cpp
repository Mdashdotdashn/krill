#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/TruncRenderNode.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("TruncRenderNode query behavior")
{
  using namespace krill;

  std::vector<RenderNodePtr> children;
  children.push_back(std::make_shared<ElementRenderNode>("1"));
  children.push_back(std::make_shared<ElementRenderNode>("2"));
  children.push_back(std::make_shared<ElementRenderNode>("3"));
  children.push_back(std::make_shared<ElementRenderNode>("4"));
  auto source = std::make_shared<HorizontalPatternRenderNode>(std::move(children));

  TruncRenderNode node(source, Fraction(3, 4));

  const auto at0 = node.query({Fraction(0), Fraction(1, 1024)});
  REQUIRE(at0.size() == 1);
  CHECK(at0[0].value == "1");
  CHECK(at0[0].wholeStart == Fraction(0));

  const auto atThreeQuarter = node.query({Fraction(3, 4), Fraction(3, 4) + Fraction(1, 1024)});
  REQUIRE(atThreeQuarter.size() == 1);
  CHECK(atThreeQuarter[0].value == "1");
  CHECK(atThreeQuarter[0].wholeStart == Fraction(3, 4));

  const auto atFiveQuarter = node.query({Fraction(5, 4), Fraction(5, 4) + Fraction(1, 1024)});
  REQUIRE(atFiveQuarter.size() == 1);
  CHECK(atFiveQuarter[0].value == "3");
  CHECK(atFiveQuarter[0].wholeStart == Fraction(5, 4));
}
