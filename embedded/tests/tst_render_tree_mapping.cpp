#include "parser/Parser.hpp"
#include "renderer/RenderTreeBuilder.hpp"

#include "renderer/nodes/AddRenderNode.hpp"
#include "renderer/nodes/ShiftRenderNode.hpp"
#include "renderer/nodes/StackRenderNode.hpp"
#include "renderer/nodes/StretchRenderNode.hpp"
#include "renderer/nodes/StructRenderNode.hpp"
#include "renderer/nodes/WeightedPatternRenderNode.hpp"

#include <third_party/catch2/catch.hpp>

namespace
{
template <typename T>
void checkTopLevelNodeType(const std::string& source)
{
  krill::Parser parser;
  rapidjson::Document parsingDocument;
  auto parseResult = parser.parse(parsingDocument, source);
  REQUIRE(parseResult.has_value());

  const auto root = krill::RenderTreeBuilder::fromJson(parseResult.value());
  REQUIRE(root != nullptr);
  REQUIRE(std::dynamic_pointer_cast<T>(root) != nullptr);
}
} // namespace

TEST_CASE("Render tree mapping uses expected top-level node classes")
{
  SECTION("add")
  {
    checkTopLevelNodeType<krill::AddRenderNode>("add (fast 2 $ '0 1') $ '2 3'");
  }

  SECTION("struct")
  {
    checkTopLevelNodeType<krill::StructRenderNode>("struct (fast 2 $ 't f') $ 'bd ~ sd ~'");
  }

  SECTION("shift")
  {
    checkTopLevelNodeType<krill::ShiftRenderNode>("rotR 0.25 $ 'bd ~ sd ~'");
  }

  SECTION("stretch")
  {
    checkTopLevelNodeType<krill::StretchRenderNode>("slow 2 $ 'a b'");
  }

  SECTION("bjorklund")
  {
    checkTopLevelNodeType<krill::WeightedPatternRenderNode>("euclid 3 8 $ 'bd'");
  }

  SECTION("stack")
  {
    checkTopLevelNodeType<krill::StackRenderNode>("stack [ 'a b', fast 2 $ '1 2' ]");
  }
}
