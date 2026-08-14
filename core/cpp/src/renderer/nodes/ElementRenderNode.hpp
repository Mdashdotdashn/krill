#pragma once

#include <map>
#include <utility>

#include "../NoteVelocity.hpp"
#include "../RenderNode.hpp"

namespace krill
{
  class ElementRenderNode final : public RenderNode
  {
  public:
    explicit ElementRenderNode(std::string value)
    : mValue(std::move(value))
    {
    }

    ElementRenderNode(std::string value, std::map<std::string, std::string> controls)
    : mValue(std::move(value)), mControls(std::move(controls))
    {
    }

    explicit ElementRenderNode(RenderNodePtr sourceNode)
    : mpSourceNode(std::move(sourceNode))
    {
    }

    ElementRenderNode(RenderNodePtr sourceNode, std::map<std::string, std::string> controls)
    : mpSourceNode(std::move(sourceNode)), mControls(std::move(controls))
    {
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end)
      {
        return {};
      }

      if (mpSourceNode)
      {
        auto fragments = mpSourceNode->query(request);
        if (mControls.empty())
        {
          return fragments;
        }

        for (auto& fragment : fragments)
        {
          fragment.controls = mergeControls(mControls, fragment.controls);
        }
        return fragments;
      }

      const auto numerator = request.start.getNumerator();
      const auto denominator = request.start.getDenominator();
      auto cycleIndex = numerator / denominator;
      if (((numerator < 0) != (denominator < 0)) && ((numerator % denominator) != 0))
      {
        cycleIndex -= 1;
      }

      const auto localStart = request.start - Fraction(cycleIndex);
      const auto localEnd = request.end - Fraction(cycleIndex);

      const Fraction wholeStart(0);
      const Fraction wholeEnd(1);
      if (localEnd <= wholeStart || localStart >= wholeEnd)
      {
        return {};
      }

      QueryFragment fragment;
      fragment.wholeStart = wholeStart + Fraction(cycleIndex);
      fragment.wholeEnd = wholeEnd + Fraction(cycleIndex);
      fragment.partStart = (localStart > wholeStart ? localStart : wholeStart) + Fraction(cycleIndex);
      fragment.partEnd = (localEnd < wholeEnd ? localEnd : wholeEnd) + Fraction(cycleIndex);
      fragment.value = mValue;
      fragment.controls = mControls;
      return {fragment};
    }

  private:
    static std::map<std::string, std::string> mergeControls(
      const std::map<std::string, std::string>& parentControls,
      const std::map<std::string, std::string>& childControls)
    {
      auto mergedControls = parentControls;
      for (const auto& entry : childControls)
      {
        mergedControls[entry.first] = entry.second;
      }

      const auto parentVelocity = note_velocity::resolveControlVelocityToMidi(parentControls, "velocity");
      const auto parentVelocityFactor = note_velocity::resolveControlVelocityToMidi(parentControls, "velocityFactor");
      const auto childVelocityFactor = note_velocity::resolveControlVelocityToMidi(childControls, "velocityFactor");

      std::optional<int> combinedVelocityFactor;

      if (parentVelocityFactor.has_value())
      {
        combinedVelocityFactor = parentVelocityFactor.value();
      }

      if (parentVelocity.has_value())
      {
        combinedVelocityFactor = combinedVelocityFactor.has_value()
          ? note_velocity::accumulateVelocityFactor(combinedVelocityFactor.value(), parentVelocity.value())
          : parentVelocity.value();
      }

      if (childVelocityFactor.has_value())
      {
        combinedVelocityFactor = combinedVelocityFactor.has_value()
          ? note_velocity::accumulateVelocityFactor(combinedVelocityFactor.value(), childVelocityFactor.value())
          : childVelocityFactor.value();
      }

      if (combinedVelocityFactor.has_value())
      {
        mergedControls["velocityFactor"] = std::to_string(combinedVelocityFactor.value());
      }

      if (!childControls.count("velocity") && parentVelocity.has_value())
      {
        mergedControls.erase("velocity");
      }

      return mergedControls;
    }

    std::string mValue;
    RenderNodePtr mpSourceNode{};
    std::map<std::string, std::string> mControls;
  };
}
