#pragma once

#include <utility>

#include "../RenderNode.hpp"

namespace krill
{
  class BjorklundRenderNode final : public RenderNode
  {
  public:
    BjorklundRenderNode(RenderNodePtr source, long pulses, long steps)
    : mpSource(std::move(source)), mPulses(pulses < 0 ? 0 : pulses), mSteps(steps <= 0 ? 1 : steps)
    {
      if (mPulses >= mSteps)
      {
        for (long i = 0; i < mSteps; i++)
        {
          mActiveSlots.push_back(i);
        }
        return;
      }

      for (long i = 0; i < mSteps; i++)
      {
        if (((i * mPulses) % mSteps) < mPulses)
        {
          mActiveSlots.push_back(i);
        }
      }
    }

    std::vector<QueryFragment> query(const QueryRequest& request) const override
    {
      if (request.start == request.end || !mpSource || mActiveSlots.empty())
      {
        return {};
      }

      const auto startNumerator = request.start.getNumerator();
      const auto startDenominator = request.start.getDenominator();
      auto startCycle = startNumerator / startDenominator;
      if (((startNumerator < 0) != (startDenominator < 0)) && ((startNumerator % startDenominator) != 0))
      {
        startCycle -= 1;
      }

      const auto endNumerator = request.end.getNumerator();
      const auto endDenominator = request.end.getDenominator();
      auto endCycle = endNumerator / endDenominator;
      if (((endNumerator < 0) != (endDenominator < 0)) && ((endNumerator % endDenominator) != 0))
      {
        endCycle -= 1;
      }

      const auto stepSize = Fraction(1, mSteps);
      std::vector<QueryFragment> fragments;

      for (long cycle = startCycle; cycle <= endCycle; cycle++)
      {
        for (size_t i = 0; i < mActiveSlots.size(); i++)
        {
          const auto slot = mActiveSlots[i];
          const auto nextSlot = (i + 1 < mActiveSlots.size())
            ? mActiveSlots[i + 1]
            : (mActiveSlots[0] + mSteps);

          const auto slotStart = Fraction(cycle) + (Fraction(slot) * stepSize);
          const auto slotEnd = Fraction(cycle) + (Fraction(nextSlot) * stepSize);
          if (request.end <= slotStart || request.start >= slotEnd)
          {
            continue;
          }

          const auto overlapStart = request.start > slotStart ? request.start : slotStart;
          const auto overlapEnd = request.end < slotEnd ? request.end : slotEnd;
          const auto slotSize = slotEnd - slotStart;

          QueryRequest localRequest;
          localRequest.start = (overlapStart - slotStart) / slotSize;
          localRequest.end = (overlapEnd - slotStart) / slotSize;

          const auto childFragments = mpSource->query(localRequest);
          for (const auto& childFragment : childFragments)
          {
            auto mapped = remapFragmentTiming(
              childFragment,
              slotStart + (childFragment.wholeStart * slotSize),
              slotStart + (childFragment.wholeEnd * slotSize),
              slotStart + (childFragment.partStart * slotSize),
              slotStart + (childFragment.partEnd * slotSize));
            fragments.push_back(mapped);
          }
        }
      }

      return fragments;
    }

  private:
    RenderNodePtr mpSource{};
    long mPulses{0};
    long mSteps{1};
    std::vector<long> mActiveSlots;
  };
}
