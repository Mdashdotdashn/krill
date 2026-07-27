#pragma once

#include "RenderNode.hpp"

#include <optional>

namespace krill
{
  class RenderTreePlayer
  {
  public:
    struct Event
    {
      Fraction time{0};
      std::vector<std::string> values;
    };

    void setTree(RenderNodePtr tree)
    {
      if (!mpTree)
      {
        mpTree = std::move(tree);
        return;
      }

      // Mid-cycle updates are applied when advance() crosses a cycle boundary.
      mpPendingTree = std::move(tree);
    }

    void reset()
    {
      if (mpPendingTree)
      {
        mpTree = mpPendingTree;
        mpPendingTree.reset();
      }
    }

    std::vector<QueryFragment> queryArc(const Fraction& start, const Fraction& end) const
    {
      if (!mpTree)
      {
        return {};
      }
      return mpTree->query(QueryRequest{start, end});
    }

    std::vector<QueryFragment> queryPointWindow(const Fraction& time) const
    {
      return queryArc(time, time + epsilon());
    }

    std::optional<Event> eventForTime(const Fraction& time) const
    {
      const auto fragments = queryPointWindow(time);
      std::vector<std::string> values;

      for (const auto& fragment : fragments)
      {
        if (fragment.wholeStart == time)
        {
          values.push_back(fragment.value);
        }
      }

      if (values.empty())
      {
        return std::nullopt;
      }

      return Event{time, std::move(values)};
    }

    std::vector<std::string> eventsForTime(const Fraction& time) const
    {
      auto event = eventForTime(time);
      if (!event)
      {
        return {};
      }
      return event->values;
    }

    Fraction advance(const Fraction& time)
    {
      const Fraction current = time;
      const Fraction nextBoundary = nextCycleBoundary(current);

      auto firstEventInRange = [this](const Fraction& startExclusive, const Fraction& endInclusive)
        -> std::optional<Fraction>
      {
        Fraction t = startExclusive + searchStep();
        t.reduce();
        while (t <= endInclusive)
        {
          const auto event = eventForTime(t);
          if (event && !event->values.empty())
          {
            return t;
          }
          t += searchStep();
          t.reduce();
        }

        return std::nullopt;
      };

      if (mpPendingTree)
      {
        const auto beforeBoundary = firstEventInRange(current, nextBoundary - epsilon());
        if (beforeBoundary)
        {
          return *beforeBoundary;
        }

        mpTree = mpPendingTree;
        mpPendingTree.reset();
        return nextBoundary;
      }

      const auto nextEvent = firstEventInRange(current, current + lookAheadCycles());
      if (nextEvent)
      {
        return *nextEvent;
      }

      return nextBoundary;
    }

  private:
    static Fraction cycleStart(const Fraction& time)
    {
      return floor(time);
    }

    static Fraction nextCycleBoundary(const Fraction& time)
    {
      return cycleStart(time) + Fraction(1);
    }

    static Fraction epsilon()
    {
      return Fraction(1, 1024);
    }

    static Fraction searchStep()
    {
      return Fraction(1, 3072);
    }

    static Fraction lookAheadCycles()
    {
      return Fraction(16);
    }

    RenderNodePtr mpTree{};
    RenderNodePtr mpPendingTree{};
  };
}
