#include "renderer/nodes/CycleRenderNode.hpp"
#include "renderer/nodes/NormalizeCycleRenderNode.hpp"

#include "testUtils.hpp"

TEST_CASE("CycleRenderNode")
{
  const auto cycle = test::simpleCycle({"1", "2", "A"});
  auto renderNode = CycleRenderNode(cycle);
  renderNode.tick();
  auto result = renderNode.render();
  test::compare(cycle, result);
}

TEST_CASE("NormalizeCycleRenderNode slices one cycle per render")
{
  const auto twoCycle = test::makeCycle(Fraction(2), {
    {Fraction(0), "a"},
    {Fraction(1), "b"}
  });

  auto child = std::make_shared<CycleRenderNode>(twoCycle);
  auto renderNode = NormalizeCycleRenderNode(child);

  renderNode.tick();
  REQUIRE(test::compare(renderNode.render(), test::simpleCycle({"a"})));

  renderNode.tick();
  REQUIRE(test::compare(renderNode.render(), test::simpleCycle({"b"})));

  renderNode.tick();
  REQUIRE(test::compare(renderNode.render(), test::simpleCycle({"a"})));
}
