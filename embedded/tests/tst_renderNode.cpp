#include "renderer/RenderNode.hpp"

#include "testUtils.hpp"

TEST_CASE("CycleRenderNode")
{
  const auto cycle = test::simpleCycle({"1", "2", "A"});
  auto renderNode = CycleRenderNode(cycle);
  renderNode.tick();
  auto result = renderNode.render();
  test::compare(cycle, result);
}
