#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/EmptyRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/nodes/StretchRenderNode.hpp"
#include "renderer/nodes/TimelinePatternRenderNode.hpp"
#include "renderer/nodes/VerticalPatternRenderNode.hpp"

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
}
