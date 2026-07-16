#pragma once

#include "RenderNode.hpp"

#include "harmony/theory/Scale.hpp"

#include <string>

namespace krill
{
class ScaleRenderNode : public RenderNode
{
public:
  ScaleRenderNode(RenderNodePtr child, std::string scaleName)
    : mpChild(std::move(child))
    , mScaleName(std::move(scaleName))
    , mIntervals(harmony::scaleIntervals(mScaleName))
  {}

  void tick() override
  {
    mpChild->tick();
  }

  Cycle render() override
  {
    const auto source = mpChild->render();
    if (!mIntervals.has_value() || mIntervals->empty())
    {
      return Cycle{};
    }

    Cycle out;
    out.length = source.length;

    const int n = static_cast<int>(mIntervals->size());
    for (const auto& event : source.events)
    {
      std::vector<std::string> values;
      values.reserve(event.values.size());

      for (const auto& v : event.values)
      {
        try
        {
          const int degree = std::stoi(v);
          const int modulo = degree % n;
          const int remainder = (degree - modulo) / n;
          const int intervalIndex = degree < 0 ? n + modulo : modulo;
          const int octave = (degree < 0 ? remainder - 1 : remainder) * 12;
          const int semitones = (*mIntervals)[static_cast<size_t>(intervalIndex)] + octave;
          values.push_back(std::to_string(semitones));
        }
        catch (...)
        {
          values.push_back("~");
        }
      }

      out.events.push_back({event.time, values});
    }

    return out;
  }

private:
  RenderNodePtr mpChild;
  std::string mScaleName;
  std::optional<std::vector<int>> mIntervals;
};

static RenderNodePtr makeScaleRenderNode(RenderNodePtr child, const std::string& scaleName)
{
  return std::make_shared<ScaleRenderNode>(std::move(child), scaleName);
}
} // namespace krill
