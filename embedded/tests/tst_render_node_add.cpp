#include "renderer/nodes/AddRenderNode.hpp"
#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"

#include <third_party/catch2/catch.hpp>

TEST_CASE("AddRenderNode query behavior")
{
  using namespace krill;

  SECTION("adds numeric values")
  {
    auto lhs = std::make_shared<ElementRenderNode>("7.5");

    std::vector<RenderNodePtr> rhsChildren;
    rhsChildren.push_back(std::make_shared<ElementRenderNode>("10"));
    rhsChildren.push_back(std::make_shared<ElementRenderNode>("11"));
    rhsChildren.push_back(std::make_shared<ElementRenderNode>("12"));
    auto rhs = std::make_shared<HorizontalPatternRenderNode>(std::move(rhsChildren));

    AddRenderNode node(lhs, rhs);
    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 3);
    CHECK(full[0].value == "17.5");
    CHECK(full[1].value == "18.5");
    CHECK(full[2].value == "19.5");
  }

  SECTION("transposes note")
  {
    auto lhs = std::make_shared<ElementRenderNode>("1");
    auto rhs = std::make_shared<ElementRenderNode>("c1");

    AddRenderNode node(lhs, rhs);
    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 1);
    CHECK(full[0].value == "C#1");
  }
}
