#pragma once

#include "RenderNode.hpp"

#include "cycle/Cycle.hpp"

#include <optional>

namespace krill
{
  class RenderTreePlayer
  {
  public:

    RenderTreePlayer()
    {}

    void setTree(RenderNodePtr pTree)
    {
      mpQueuedTree = pTree;
    }

    // Advance in the current cycle and returns the position
    // where eventForTime should be called. We prepare the next
    // event to broadcast when @eventForTime will be called.
    // (An empty event is always sent at the end of the 
    // current cycle).
    Fraction advance(const Fraction& currentTime)
    {
      const auto cycleTimeAndStart = calcCycleTimeAndStart(currentTime);
      const auto cycleTime = cycleTimeAndStart.first;
      const auto cycleStart = cycleTimeAndStart.second;

      // Do we have an event to broadcast ?
      const std::optional<Cycle::Event> oNextEvent = mpCurrentTree ? findEventAfter(mCurrentCycle, cycleTime) : std::optional<Cycle::Event>{};
      if (oNextEvent)
      {
        const auto position = cycleStart + oNextEvent->time;
        mQueuedEvent = Cycle::Event{ position, oNextEvent->values };
      }
      else
      {
        mShouldReset = true;
        const auto cycleLength = mCurrentCycle.length;
        const auto position = cycleStart + cycleLength;
        mQueuedEvent = Cycle::Event{ position, {} };
      }
      return mQueuedEvent.time;
    }

    std::optional<Cycle::Event> eventForTime(const Fraction& currentTime)
    {
      // In the case we have a reset, there could be an even waiting for
      // us at the beginning of the next sequence, in which case we'll
      // update mQueuedEvent to contain the values associated to that event.
      if (mShouldReset)
      {
        if (mpQueuedTree)
        {
          mpCurrentTree = mpQueuedTree;
          mpQueuedTree = nullptr;
        }

        mShouldReset = false;

        if (mpCurrentTree)
        {
          mpCurrentTree->tick();
          mCurrentCycle = mpCurrentTree->render();
          mCurrentCycleOffset = currentTime;
          if (mCurrentCycle.events.size() > 0)
          {
            const auto firstEvent = mCurrentCycle.events[0];
            mQueuedEvent.values.clear();
            if (firstEvent.time == Fraction(0))
            {
              mQueuedEvent.values = firstEvent.values;
            }
          }
          else
          {
            clearQueuedEvent();
          }
        }
        else
        {
          mCurrentCycle = Cycle{ 1, {} };
          clearQueuedEvent();
        }
      }

      if (mQueuedEvent.values.size() > 0)
      {
        if (mQueuedEvent.time == currentTime)
        {
          return mQueuedEvent;
        }
      }
      return {};
    }

  private:

    std::pair<Fraction, Fraction>  calcCycleTimeAndStart(const Fraction& time)
    {
      const auto localTime = time - mCurrentCycleOffset;
      const auto cycleLength = mCurrentCycle.length;
      const auto cycleTime = localTime % cycleLength;
      const auto cycleStart = floor(localTime / cycleLength) * cycleLength + mCurrentCycleOffset;
      return { Fraction(cycleTime), cycleStart };
    }

    void clearQueuedEvent()
    {
      mQueuedEvent.values = {};
    }

    RenderNodePtr mpCurrentTree{};
    RenderNodePtr mpQueuedTree{};
    bool mShouldReset{ false };
    Cycle mCurrentCycle{ 1, {} };
    Fraction mCurrentCycleOffset{ 0 };
    Cycle::Event mQueuedEvent;
  };
} // namespace krill
