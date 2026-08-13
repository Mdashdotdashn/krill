#include "renderer/nodes/ElementRenderNode.hpp"
#include "renderer/nodes/EmptyRenderNode.hpp"
#include "renderer/nodes/HorizontalPatternRenderNode.hpp"
#include "renderer/RenderTreeBuilder.hpp"
#include "parser/Parser.hpp"

#include "../third_party/catch2/catch.hpp"

namespace
{
class SpyRenderNode final : public krill::RenderNode
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

TEST_CASE("Render query request and fragments contract")
{
  using namespace krill;

  SECTION("EmptyRenderNode returns no fragments")
  {
    EmptyRenderNode node;
    const auto fragments = node.query({Fraction(0), Fraction(1)});
    CHECK(fragments.empty());
  }

  SECTION("ElementRenderNode fills full fragment metadata")
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
    CHECK(partial[0].wholeStart == Fraction(0));
    CHECK(partial[0].wholeEnd == Fraction(1));
    CHECK(partial[0].partStart == Fraction(1, 4));
    CHECK(partial[0].partEnd == Fraction(3, 4));

    const auto noWidth = node.query({Fraction(1), Fraction(1)});
    CHECK(noWidth.empty());
  }

  SECTION("ElementRenderNode forwards QueryRequest to nested source")
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

  SECTION("Parsed velocity becomes fragment controls through render tree")
  {
    Parser parser;
    rapidjson::Document parsingDocument;
    auto result = parser.parse(parsingDocument, "'bd:0.8'");
    REQUIRE(result.has_value());

    auto tree = RenderTreeBuilder::fromJson(result.value());
    const auto fragments = tree->query({Fraction(0), Fraction(1)});

    REQUIRE(fragments.size() == 1);
    REQUIRE(fragments[0].controls.count("velocity") == 1);
    CHECK(fragments[0].controls.at("velocity") == "0.8");
  }

  SECTION("Horizontal remapping preserves controls metadata")
  {
    std::map<std::string, std::string> controls{{"velocity", "100"}};
    auto child = std::make_shared<ElementRenderNode>(std::string("snare"), controls);
    HorizontalPatternRenderNode node({child}, {Fraction(1)});

    const auto fragments = node.query({Fraction(0), Fraction(1)});
    REQUIRE(fragments.size() == 1);
    REQUIRE(fragments[0].controls.count("velocity") == 1);
    CHECK(fragments[0].controls.at("velocity") == "100");
  }

  SECTION("Nested velocity controls compose through inherited factor")
  {
    std::map<std::string, std::string> controls{{"velocity", "100"}};

    class VelocitySpyRenderNode final : public RenderNode
    {
    public:
      std::vector<QueryFragment> query(const QueryRequest& request) const override
      {
        QueryFragment fragment;
        fragment.wholeStart = Fraction(0);
        fragment.wholeEnd = Fraction(1);
        fragment.partStart = request.start;
        fragment.partEnd = request.end;
        fragment.value = "nested";
        fragment.controls = {{"velocity", "80"}};
        return {fragment};
      }
    };

    auto child = std::make_shared<VelocitySpyRenderNode>();
    ElementRenderNode node(child, controls);

    const auto fragments = node.query({Fraction(0), Fraction(1)});
    REQUIRE(fragments.size() == 1);
    REQUIRE(fragments[0].controls.count("velocity") == 1);
    REQUIRE(fragments[0].controls.count("velocityFactor") == 1);
    CHECK(fragments[0].controls.at("velocity") == "80");
    CHECK(fragments[0].controls.at("velocityFactor") == "100");
  }
}