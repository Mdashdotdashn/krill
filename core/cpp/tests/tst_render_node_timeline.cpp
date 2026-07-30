#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/StretchRenderNode.hpp"
#include "renderer/nodes/TimelinePatternRenderNode.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("TimelinePatternRenderNode query behavior")
{
  using namespace krill;

  SECTION("alternates by cycle")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("3"));
    children.push_back(std::make_shared<ElementRenderNode>("4"));

    TimelinePatternRenderNode node(std::move(children));

    const auto cycle0 = node.query({Fraction(2, 3), Fraction(2, 3) + Fraction(1, 1024)});
    REQUIRE(cycle0.size() == 1);
    CHECK(cycle0[0].value == "3");
    CHECK(cycle0[0].wholeStart == Fraction(0));
    CHECK(cycle0[0].wholeEnd == Fraction(1));
    CHECK(cycle0[0].partStart == Fraction(2, 3));

    const auto cycle1 = node.query({Fraction(5, 3), Fraction(5, 3) + Fraction(1, 1024)});
    REQUIRE(cycle1.size() == 1);
    CHECK(cycle1[0].value == "4");
    CHECK(cycle1[0].wholeStart == Fraction(1));
    CHECK(cycle1[0].wholeEnd == Fraction(2));
    CHECK(cycle1[0].partStart == Fraction(5, 3));
  }

  SECTION("concatenates child spans")
  {
    std::vector<RenderNodePtr> leftChildren;
    leftChildren.push_back(std::make_shared<ElementRenderNode>("1"));
    leftChildren.push_back(std::make_shared<ElementRenderNode>("12"));
    auto left = std::make_shared<StretchRenderNode>(
      std::make_shared<HorizontalPatternRenderNode>(std::move(leftChildren)),
      Fraction(2));

    std::vector<RenderNodePtr> rightChildren;
    rightChildren.push_back(std::make_shared<ElementRenderNode>("5"));
    rightChildren.push_back(std::make_shared<ElementRenderNode>("17"));
    auto right = std::make_shared<StretchRenderNode>(
      std::make_shared<HorizontalPatternRenderNode>(std::move(rightChildren)),
      Fraction(4));

    TimelinePatternRenderNode node({left, right});

    const auto at1 = node.query({Fraction(1), Fraction(1) + Fraction(1, 1024)});
    REQUIRE(at1.size() == 1);
    CHECK(at1[0].value == "12");
    CHECK(at1[0].wholeStart == Fraction(1));
    CHECK(at1[0].wholeEnd == Fraction(2));

    const auto at2 = node.query({Fraction(2), Fraction(2) + Fraction(1, 1024)});
    REQUIRE(at2.size() == 1);
    CHECK(at2[0].value == "5");
    CHECK(at2[0].wholeStart == Fraction(2));
    CHECK(at2[0].wholeEnd == Fraction(4));

    const auto at4 = node.query({Fraction(4), Fraction(4) + Fraction(1, 1024)});
    REQUIRE(at4.size() == 1);
    CHECK(at4[0].value == "17");
    CHECK(at4[0].wholeStart == Fraction(4));
    CHECK(at4[0].wholeEnd == Fraction(6));
  }
}
