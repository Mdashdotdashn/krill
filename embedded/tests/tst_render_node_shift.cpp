#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/ShiftRenderNode.hpp"
#include "renderer/nodes/TimelinePatternRenderNode.hpp"
#include "renderer/RenderTreeBuilder.hpp"
#include "parser/Parser.hpp"

#include <third_party/catch2/catch.hpp>

TEST_CASE("ShiftRenderNode query behavior")
{
  using namespace krill;

  SECTION("fixed amount")
  {
    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("bd"));
    children.push_back(std::make_shared<ElementRenderNode>("~"));
    children.push_back(std::make_shared<ElementRenderNode>("sd"));
    children.push_back(std::make_shared<ElementRenderNode>("~"));
    auto source = std::make_shared<HorizontalPatternRenderNode>(std::move(children));

    ShiftRenderNode right(source, Fraction("1/8"), 1);
    const auto rightAt = right.query({Fraction(1, 8), Fraction(1, 8) + Fraction(1, 1024)});
    REQUIRE(rightAt.size() == 1);
    CHECK(rightAt[0].value == "bd");
    CHECK(rightAt[0].wholeStart == Fraction(1, 8));
    CHECK(rightAt[0].wholeEnd == Fraction(3, 8));

    ShiftRenderNode left(source, Fraction("1/8"), -1);
    const auto leftAt = left.query({Fraction(1, 8), Fraction(1, 8) + Fraction(1, 1024)});
    REQUIRE(leftAt.size() == 1);
    CHECK(leftAt[0].value == "~");
  }

  SECTION("dynamic amount source")
  {
    std::vector<RenderNodePtr> amountChildren;
    amountChildren.push_back(std::make_shared<ElementRenderNode>("0"));
    amountChildren.push_back(std::make_shared<ElementRenderNode>("0.125"));
    auto amountTimeline = std::make_shared<TimelinePatternRenderNode>(std::move(amountChildren));
    auto amountSource = std::make_shared<ElementRenderNode>(amountTimeline);

    std::vector<RenderNodePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("bd"));
    children.push_back(std::make_shared<ElementRenderNode>("~"));
    children.push_back(std::make_shared<ElementRenderNode>("sd"));
    children.push_back(std::make_shared<ElementRenderNode>("~"));
    auto source = std::make_shared<HorizontalPatternRenderNode>(std::move(children));

    ShiftRenderNode node(source, amountSource, -1);

    const auto amountCycle1 = amountSource->query({Fraction(9, 8), Fraction(9, 8) + Fraction(1, 1024)});
    REQUIRE(amountCycle1.size() == 1);
    CHECK(amountCycle1[0].value == "0.125");

    const auto amountCycle0 = amountSource->query({Fraction(0), Fraction(1, 1024)});
    REQUIRE(amountCycle0.size() == 1);
    CHECK(amountCycle0[0].value == "0");
    CHECK(amountCycle0[0].wholeStart == Fraction(0));

    const auto sourceCycle0 = source->query({Fraction(0), Fraction(1, 1024)});
    REQUIRE(sourceCycle0.size() == 1);
    CHECK(sourceCycle0[0].value == "bd");

    const auto cycle0 = node.query({Fraction(0), Fraction(1, 1024)});
    REQUIRE(cycle0.size() == 1);
    CHECK(cycle0[0].value == "bd");
    CHECK(cycle0[0].wholeStart == Fraction(0));

    const auto cycle1 = node.query({Fraction(9, 8), Fraction(9, 8) + Fraction(1, 1024)});
    REQUIRE(cycle1.size() == 1);
    CHECK(cycle1[0].value == "~");
    CHECK(cycle1[0].wholeStart == Fraction(9, 8));
  }

  SECTION("via parser and builder")
  {
    rapidjson::Document document;
    Parser parser;
    auto parsed = parser.parse(document, "rotL '<0 0.125>' $ 'bd ~ sd ~'");
    REQUIRE(parsed.has_value());

    auto tree = RenderTreeBuilder::fromJson(parsed.value());

    const auto cycle0 = tree->query({Fraction(0), Fraction(1, 1024)});
    REQUIRE(cycle0.size() == 1);
    CHECK(cycle0[0].value == "bd");
    CHECK(cycle0[0].wholeStart == Fraction(0));

    const auto cycle1 = tree->query({Fraction(9, 8), Fraction(9, 8) + Fraction(1, 1024)});
    REQUIRE(cycle1.size() == 1);
    CHECK(cycle1[0].value == "~");
    CHECK(cycle1[0].wholeStart == Fraction(9, 8));
  }
}
