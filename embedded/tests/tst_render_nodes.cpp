#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/EmptyRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
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
}
