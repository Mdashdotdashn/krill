#include "renderer/nodes/BjorklundRenderNode.hpp"
#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("BjorklundRenderNode query behavior")
{
  using namespace krill;

  std::vector<RenderNodePtr> children;
  children.push_back(std::make_shared<ElementRenderNode>("bd"));
  children.push_back(std::make_shared<ElementRenderNode>("sd"));
  auto source = std::make_shared<HorizontalPatternRenderNode>(std::move(children));
  BjorklundRenderNode node(source, 2, 8);

  const auto at0 = node.query({Fraction(0), Fraction(1, 1024)});
  REQUIRE(at0.size() == 1);
  CHECK(at0[0].value == "bd");
  CHECK(at0[0].wholeStart == Fraction(0));
  CHECK(at0[0].wholeEnd == Fraction(1, 4));

  const auto atQuarter = node.query({Fraction(1, 4), Fraction(1, 4) + Fraction(1, 1024)});
  REQUIRE(atQuarter.size() == 1);
  CHECK(atQuarter[0].value == "sd");
  CHECK(atQuarter[0].wholeStart == Fraction(1, 4));
  CHECK(atQuarter[0].wholeEnd == Fraction(1, 2));

  const auto atHalf = node.query({Fraction(1, 2), Fraction(1, 2) + Fraction(1, 1024)});
  REQUIRE(atHalf.size() == 1);
  CHECK(atHalf[0].value == "bd");
  CHECK(atHalf[0].wholeStart == Fraction(1, 2));
  CHECK(atHalf[0].wholeEnd == Fraction(3, 4));

  const auto atThreeQuarter = node.query({Fraction(3, 4), Fraction(3, 4) + Fraction(1, 1024)});
  REQUIRE(atThreeQuarter.size() == 1);
  CHECK(atThreeQuarter[0].value == "sd");
  CHECK(atThreeQuarter[0].wholeStart == Fraction(3, 4));
  CHECK(atThreeQuarter[0].wholeEnd == Fraction(1));
}
