#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/VerticalPatternRenderNode.hpp"
#include "renderNodeTestUtils.hpp"

#include "../third_party/catch2/catch.hpp"

TEST_CASE("HorizontalPatternRenderNode query behavior")
{
  using namespace krill;

  SECTION("query full and partial ranges")
  {
    HorizontalPatternRenderNode node({"a", "b", "c"});

    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 3);
    CHECK(krill::tests::values(full) == std::vector<std::string>{"a", "b", "c"});

    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 3));
    CHECK(full[0].partStart == Fraction(0));
    CHECK(full[0].partEnd == Fraction(1, 3));

    CHECK(full[1].wholeStart == Fraction(1, 3));
    CHECK(full[1].wholeEnd == Fraction(2, 3));
    CHECK(full[2].wholeStart == Fraction(2, 3));
    CHECK(full[2].wholeEnd == Fraction(1));

    const auto middleOnly = node.query({Fraction(1, 3), Fraction(2, 3)});
    REQUIRE(middleOnly.size() == 1);
    CHECK(middleOnly[0].value == "b");
    CHECK(middleOnly[0].partStart == Fraction(1, 3));
    CHECK(middleOnly[0].partEnd == Fraction(2, 3));
  }

  SECTION("constructor from values")
  {
    HorizontalPatternRenderNode node({"a", "b", "c"});
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 3);
    CHECK(krill::tests::values(full) == std::vector<std::string>{"a", "b", "c"});
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 3));
    CHECK(full[1].wholeStart == Fraction(1, 3));
    CHECK(full[1].wholeEnd == Fraction(2, 3));
    CHECK(full[2].wholeStart == Fraction(2, 3));
    CHECK(full[2].wholeEnd == Fraction(1));
  }

  SECTION("constructor children defaults weights")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("a"));
    children.push_back(std::make_shared<ElementRenderNode>("b"));

    HorizontalPatternRenderNode node(std::move(children));
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 2);
    CHECK(krill::tests::values(full) == std::vector<std::string>{"a", "b"});
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 2));
    CHECK(full[1].wholeStart == Fraction(1, 2));
    CHECK(full[1].wholeEnd == Fraction(1));
  }

  SECTION("constructor children explicit weights")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("a"));
    children.push_back(std::make_shared<ElementRenderNode>("b"));

    HorizontalPatternRenderNode node(std::move(children), {Fraction(3), Fraction(1)});
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 2);
    CHECK(krill::tests::values(full) == std::vector<std::string>{"a", "b"});
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(3, 4));
    CHECK(full[1].wholeStart == Fraction(3, 4));
    CHECK(full[1].wholeEnd == Fraction(1));
  }

  SECTION("constructor children weight size mismatch")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("a"));
    children.push_back(std::make_shared<ElementRenderNode>("b"));

    HorizontalPatternRenderNode node(std::move(children), {Fraction(3)});
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 2);
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 2));
    CHECK(full[1].wholeStart == Fraction(1, 2));
    CHECK(full[1].wholeEnd == Fraction(1));
  }

  SECTION("constructor children non-positive weights normalize")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("a"));
    children.push_back(std::make_shared<ElementRenderNode>("b"));

    HorizontalPatternRenderNode node(std::move(children), {Fraction(0), Fraction(-2)});
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 2);
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 2));
    CHECK(full[1].wholeStart == Fraction(1, 2));
    CHECK(full[1].wholeEnd == Fraction(1));
  }

  SECTION("delegates nested child query")
  {
    std::vector<RenderNodePtr> nested;
    nested.push_back(std::make_shared<ElementRenderNode>("6"));
    nested.push_back(std::make_shared<ElementRenderNode>("C4"));

    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("1"));
    children.push_back(std::make_shared<ElementRenderNode>("2"));
    children.push_back(std::make_shared<VerticalPatternRenderNode>(std::move(nested)));

    HorizontalPatternRenderNode node(std::move(children));
    const auto result = node.query({Fraction(2, 3), Fraction(1)});

    REQUIRE(result.size() == 2);
    CHECK(krill::tests::values(result) == std::vector<std::string>{"6", "C4"});
    CHECK(result[0].wholeStart == Fraction(2, 3));
    CHECK(result[0].wholeEnd == Fraction(1));
    CHECK(result[0].partStart == Fraction(2, 3));
    CHECK(result[0].partEnd == Fraction(1));
    CHECK(result[1].wholeStart == Fraction(2, 3));
    CHECK(result[1].wholeEnd == Fraction(1));
    CHECK(result[1].partStart == Fraction(2, 3));
    CHECK(result[1].partEnd == Fraction(1));
  }

  SECTION("weighted subdivision boundaries")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("bd"));
    children.push_back(std::make_shared<ElementRenderNode>("sd"));

    HorizontalPatternRenderNode node(std::move(children), {Fraction(3), Fraction(1)});

    const auto at0 = node.query({Fraction(0), Fraction(1, 1024)});
    REQUIRE(at0.size() == 1);
    CHECK(at0[0].value == "bd");
    CHECK(at0[0].wholeStart == Fraction(0));
    CHECK(at0[0].wholeEnd == Fraction(3, 4));

    const auto atThreeQuarters = node.query({Fraction(3, 4), Fraction(3, 4) + Fraction(1, 1024)});
    REQUIRE(atThreeQuarters.size() == 1);
    CHECK(atThreeQuarters[0].value == "sd");
    CHECK(atThreeQuarters[0].wholeStart == Fraction(3, 4));
    CHECK(atThreeQuarters[0].wholeEnd == Fraction(1));
  }
}
