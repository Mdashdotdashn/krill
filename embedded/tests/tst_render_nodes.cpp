#include "renderer/nodes/AddRenderNode.hpp"
#include "renderer/nodes/BjorklundRenderNode.hpp"
#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/EmptyRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/ScaleRenderNode.hpp"
#include "renderer/nodes/ShiftRenderNode.hpp"
#include "renderer/nodes/StretchRenderNode.hpp"
#include "renderer/nodes/StructRenderNode.hpp"
#include "renderer/nodes/TimelinePatternRenderNode.hpp"
#include "renderer/nodes/VerticalPatternRenderNode.hpp"
#include "parser/Parser.hpp"

#include <third_party/catch2/catch.hpp>

namespace
{
std::vector<std::string> values(const std::vector<krill::QueryFragment>& fragments)
{
  std::vector<std::string> out;
  out.reserve(fragments.size());
  for (const auto& fragment : fragments)
  {
    out.push_back(fragment.value);
  }
  return out;
}

class SpyRenderNode final : public krill::RenderTree
{
public:
  std::vector<krill::QueryFragment> query(const krill::QueryRequest& request) const override
  {
    mWasCalled = true;
    mLastRequest = request;

    krill::QueryFragment fragment;
    fragment.wholeStart = Fraction(0);
    fragment.wholeEnd = Fraction(1);
    fragment.partStart = request.start;
    fragment.partEnd = request.end;
    fragment.value = "nested";
    return {fragment};
  }

  mutable bool mWasCalled{false};
  mutable krill::QueryRequest mLastRequest{};
};
} // namespace

TEST_CASE("Render nodes query behavior")
{
  using namespace krill;

  SECTION("EmptyRenderNode")
  {
    EmptyRenderNode node;
    const auto fragments = node.query({Fraction(0), Fraction(1)});
    CHECK(fragments.empty());
  }

  SECTION("ElementRenderNode literal")
  {
    ElementRenderNode node("kick");

    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 1);
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1));
    CHECK(full[0].partStart == Fraction(0));
    CHECK(full[0].partEnd == Fraction(1));
    CHECK(full[0].value == "kick");

    const auto partial = node.query({Fraction(1, 4), Fraction(3, 4)});
    REQUIRE(partial.size() == 1);
    CHECK(partial[0].partStart == Fraction(1, 4));
    CHECK(partial[0].partEnd == Fraction(3, 4));

    const auto noWidth = node.query({Fraction(1), Fraction(1)});
    CHECK(noWidth.empty());
  }

  SECTION("ElementRenderNode delegates nested source")
  {
    auto spy = std::make_shared<SpyRenderNode>();
    ElementRenderNode node(spy);

    const auto result = node.query({Fraction(1, 8), Fraction(3, 8)});
    REQUIRE(spy->mWasCalled);
    CHECK(spy->mLastRequest.start == Fraction(1, 8));
    CHECK(spy->mLastRequest.end == Fraction(3, 8));
    REQUIRE(result.size() == 1);
    CHECK(result[0].value == "nested");
    CHECK(result[0].partStart == Fraction(1, 8));
    CHECK(result[0].partEnd == Fraction(3, 8));
  }

  SECTION("HorizontalPatternRenderNode")
  {
    HorizontalPatternRenderNode node({"a", "b", "c"});

    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 3);
    CHECK(values(full) == std::vector<std::string>{"a", "b", "c"});

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

  SECTION("HorizontalPatternRenderNode constructor values")
  {
    HorizontalPatternRenderNode node({"a", "b", "c"});
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 3);
    CHECK(values(full) == std::vector<std::string>{"a", "b", "c"});
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 3));
    CHECK(full[1].wholeStart == Fraction(1, 3));
    CHECK(full[1].wholeEnd == Fraction(2, 3));
    CHECK(full[2].wholeStart == Fraction(2, 3));
    CHECK(full[2].wholeEnd == Fraction(1));
  }

  SECTION("HorizontalPatternRenderNode constructor children defaults weights")
  {
    std::vector<RenderTreePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("a"));
    children.push_back(std::make_shared<ElementRenderNode>("b"));

    HorizontalPatternRenderNode node(std::move(children));
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 2);
    CHECK(values(full) == std::vector<std::string>{"a", "b"});
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(1, 2));
    CHECK(full[1].wholeStart == Fraction(1, 2));
    CHECK(full[1].wholeEnd == Fraction(1));
  }

  SECTION("HorizontalPatternRenderNode constructor children with explicit weights")
  {
    std::vector<RenderTreePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("a"));
    children.push_back(std::make_shared<ElementRenderNode>("b"));

    HorizontalPatternRenderNode node(std::move(children), {Fraction(3), Fraction(1)});
    const auto full = node.query({Fraction(0), Fraction(1)});

    REQUIRE(full.size() == 2);
    CHECK(values(full) == std::vector<std::string>{"a", "b"});
    CHECK(full[0].wholeStart == Fraction(0));
    CHECK(full[0].wholeEnd == Fraction(3, 4));
    CHECK(full[1].wholeStart == Fraction(3, 4));
    CHECK(full[1].wholeEnd == Fraction(1));
  }

  SECTION("HorizontalPatternRenderNode constructor children weight size mismatch")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("HorizontalPatternRenderNode constructor non-positive weights normalize")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("HorizontalPatternRenderNode delegates nested child query")
  {
    std::vector<RenderTreePtr> nested;
    nested.push_back(std::make_shared<ElementRenderNode>("6"));
    nested.push_back(std::make_shared<ElementRenderNode>("C4"));

    std::vector<RenderTreePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("1"));
    children.push_back(std::make_shared<ElementRenderNode>("2"));
    children.push_back(std::make_shared<VerticalPatternRenderNode>(std::move(nested)));

    HorizontalPatternRenderNode node(std::move(children));
    const auto result = node.query({Fraction(2, 3), Fraction(1)});

    REQUIRE(result.size() == 2);
    CHECK(values(result) == std::vector<std::string>{"6", "C4"});
    CHECK(result[0].wholeStart == Fraction(2, 3));
    CHECK(result[0].wholeEnd == Fraction(1));
    CHECK(result[0].partStart == Fraction(2, 3));
    CHECK(result[0].partEnd == Fraction(1));
    CHECK(result[1].wholeStart == Fraction(2, 3));
    CHECK(result[1].wholeEnd == Fraction(1));
    CHECK(result[1].partStart == Fraction(2, 3));
    CHECK(result[1].partEnd == Fraction(1));
  }

  SECTION("VerticalPatternRenderNode")
  {
    std::vector<RenderTreePtr> children;
    children.push_back(std::make_shared<ElementRenderNode>("left"));
    children.push_back(nullptr);
    children.push_back(std::make_shared<ElementRenderNode>("right"));

    VerticalPatternRenderNode node(std::move(children));

    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 2);
    CHECK(values(full) == std::vector<std::string>{"left", "right"});

    const auto noWidth = node.query({Fraction(1, 2), Fraction(1, 2)});
    CHECK(noWidth.empty());
  }

  SECTION("TimelinePatternRenderNode alternates by cycle")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("StretchRenderNode")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("BjorklundRenderNode")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("StructRenderNode")
  {
    std::vector<RenderTreePtr> maskChildren;
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

  SECTION("AddRenderNode")
  {
    auto lhs = std::make_shared<ElementRenderNode>("7.5");

    std::vector<RenderTreePtr> rhsChildren;
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

  SECTION("AddRenderNode transposes note")
  {
    auto lhs = std::make_shared<ElementRenderNode>("1");
    auto rhs = std::make_shared<ElementRenderNode>("c1");

    AddRenderNode node(lhs, rhs);
    const auto full = node.query({Fraction(0), Fraction(1)});
    REQUIRE(full.size() == 1);
    CHECK(full[0].value == "C#1");
  }

  SECTION("ScaleRenderNode")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("ShiftRenderNode")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("ShiftRenderNode with dynamic amount source")
  {
    std::vector<RenderTreePtr> amountChildren;
    amountChildren.push_back(std::make_shared<ElementRenderNode>("0"));
    amountChildren.push_back(std::make_shared<ElementRenderNode>("0.125"));
    auto amountTimeline = std::make_shared<TimelinePatternRenderNode>(std::move(amountChildren));
    auto amountSource = std::make_shared<ElementRenderNode>(amountTimeline);

    std::vector<RenderTreePtr> children;
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

  SECTION("ShiftRenderNode via parser and builder")
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

  SECTION("HorizontalPatternRenderNode weights")
  {
    std::vector<RenderTreePtr> children;
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

  SECTION("TimelinePatternRenderNode concatenates child spans")
  {
    std::vector<RenderTreePtr> leftChildren;
    leftChildren.push_back(std::make_shared<ElementRenderNode>("1"));
    leftChildren.push_back(std::make_shared<ElementRenderNode>("12"));
    auto left = std::make_shared<StretchRenderNode>(
      std::make_shared<HorizontalPatternRenderNode>(std::move(leftChildren)),
      Fraction(2));

    std::vector<RenderTreePtr> rightChildren;
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
