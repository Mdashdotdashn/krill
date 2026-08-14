#pragma once

#include <map>
#include <stdexcept>
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
      if (mControls.count("velocity") > 0)
      {
        throw std::invalid_argument("Nested ElementRenderNode controls must use velocityFactor, not velocity.");
      }

      if (mControls.count("velocityFactor") > 0)
      {
        const auto factor = note_velocity::resolveControlVelocityFactor(mControls, "velocityFactor");
        if (!factor.has_value())
        {
          throw std::invalid_argument("Nested ElementRenderNode velocityFactor must be within [0, 1].");
        }
        mControls["velocityFactor"] = note_velocity::formatVelocityFactor(factor.value());
      }
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

      const auto parentVelocityFactor = note_velocity::resolveControlVelocityFactor(parentControls, "velocityFactor");
      const auto childVelocityFactor = note_velocity::resolveControlVelocityFactor(childControls, "velocityFactor");

      std::optional<double> combinedVelocityFactor;
      auto accumulateContribution = [&combinedVelocityFactor](const std::optional<double>& contribution)
      {
        if (!contribution.has_value())
        {
          return;
        }

        combinedVelocityFactor = combinedVelocityFactor.has_value()
          ? note_velocity::accumulateVelocityFactor(combinedVelocityFactor.value(), contribution.value())
          : contribution.value();
      };

      accumulateContribution(parentVelocityFactor);
      accumulateContribution(childVelocityFactor);

      if (combinedVelocityFactor.has_value())
      {
        mergedControls["velocityFactor"] = note_velocity::formatVelocityFactor(combinedVelocityFactor.value());
      }

      return mergedControls;
    }

    std::string mValue;
    RenderNodePtr mpSourceNode{};
    std::map<std::string, std::string> mControls;
  };
}
