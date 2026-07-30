#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/StructRenderNode.hpp"

#include <third_party/catch2/catch.hpp>

TEST_CASE("StructRenderNode query behavior")
{
  using namespace krill;

  std::vector<RenderNodePtr> maskChildren;
  maskChildren.push_back(std::make_shared<ElementRenderNode>("t"));
  maskChildren.push_back(std::make_shared<ElementRenderNode>("f"));
  maskChildren.push_back(std::make_shared<ElementRenderNode>("f"));
  maskChildren.push_back(std::make_shared<ElementRenderNode>("t"));
  auto mask = std::make_shared<HorizontalPatternRenderNode>(std::move(maskChildren));

  auto source = std::make_shared<ElementRenderNode>("bd");
  StructRenderNode node(mask, source);

  const auto at0 = node.query({Fraction(0), Fraction(1, 1024)});
  REQUIRE(at0.size() == 1);
  CHECK(at0[0].value == "bd");

  const auto atQuarter = node.query({Fraction(1, 4), Fraction(1, 4) + Fraction(1, 1024)});
  REQUIRE(atQuarter.size() == 1);
  CHECK(atQuarter[0].value == "~");
  CHECK(atQuarter[0].wholeStart == Fraction(1, 4));
  CHECK(atQuarter[0].wholeEnd == Fraction(1, 2));

  const auto atHalf = node.query({Fraction(1, 2), Fraction(1, 2) + Fraction(1, 1024)});
  REQUIRE(atHalf.size() == 1);
  CHECK(atHalf[0].value == "~");

  const auto atThreeQuarter = node.query({Fraction(3, 4), Fraction(3, 4) + Fraction(1, 1024)});
  REQUIRE(atThreeQuarter.size() == 1);
  CHECK(atThreeQuarter[0].value == "bd");
}
