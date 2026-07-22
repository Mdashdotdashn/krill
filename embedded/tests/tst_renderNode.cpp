#include "renderer/nodes/CycleRenderNode.hpp"
#include "renderer/nodes/NormalizeCycleRenderNode.hpp"
#include "renderer/nodes/ShiftRenderNode.hpp"

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

TEST_CASE("ShiftRenderNode rotates scalar offsets")
{
  const auto base = test::simpleCycle({"bd", "~", "sd", "~"});
  auto child = std::make_shared<CycleRenderNode>(base);
  auto renderNode = ShiftRenderNode(child, Fraction(1, 8));

  renderNode.tick();
  const auto shifted = renderNode.render();

  const auto expected = test::makeCycle(Fraction(1), {
    {Fraction(1, 8), "bd"},
    {Fraction(3, 8), "~"},
    {Fraction(5, 8), "sd"},
    {Fraction(7, 8), "~"},
  });

  REQUIRE(test::compare(shifted, expected));
}
