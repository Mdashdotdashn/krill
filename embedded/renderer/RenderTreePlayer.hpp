#pragma once

#include "RenderNode.hpp"

#include <optional>

namespace krill
{
  class RenderTreePlayer
  {
  public:
    void setTree(RenderNodePtr tree)
    {
      if (!mpTree)
      {
        mpTree = std::move(tree);
        return;
      }

      // Mid-cycle updates are applied when nextOnsetTimeFrom() crosses a cycle boundary.
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

    // Preferred payload API: returns the values array at the given time, or {} if none.
    std::vector<std::string> eventsAtTime(const Fraction& time) const
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

      return values;
    }

    // Preferred scheduler API: returns the next onset time strictly after the given time.
    //
    // Uses a point query (queryPointWindow) at each step rather than a large arc
    // query, so time-varying render-node parameters are always resolved at the
    // correct point in time. Within each step, wholeEnd from the returned fragment
    // is used to jump directly to the start of the next slot, avoiding fixed-size
    // step scanning.
    Fraction nextOnsetTimeFrom(const Fraction& time)
    {
      const Fraction current = time;
      const Fraction nextBoundary = nextCycleBoundary(current);
      const Fraction searchEnd = mpPendingTree ? nextBoundary : (current + lookAheadCycles());

      Fraction t = current;

      while (t < searchEnd)
      {
        const auto fragments = queryPointWindow(t);

        if (fragments.empty())
        {
          t = nextCycleBoundary(t);
          continue;
        }

        std::optional<Fraction> nextOnset;
        std::optional<Fraction> nextT;

        for (const auto& f : fragments)
        {
          Fraction onset = f.wholeStart;
          onset.reduce();

          // Onset strictly after current and within the search range.
          if (onset > current && onset <= searchEnd)
          {
            if (!nextOnset || onset < *nextOnset)
            {
              nextOnset = onset;
            }
          }

          // wholeEnd gives the exact start of the next slot — use it to advance t.
          Fraction end = f.wholeEnd;
          end.reduce();
          if (end > t && (!nextT || end < *nextT))
          {
            nextT = end;
          }
        }

        if (nextOnset)
        {
          return *nextOnset;
        }

        t = nextT ? *nextT : nextCycleBoundary(t);
        t.reduce();
      }

      if (mpPendingTree)
      {
        mpTree = mpPendingTree;
        mpPendingTree.reset();
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

    static Fraction lookAheadCycles()
    {
      return Fraction(16);
    }

    RenderNodePtr mpTree{};
    RenderNodePtr mpPendingTree{};
  };
}
