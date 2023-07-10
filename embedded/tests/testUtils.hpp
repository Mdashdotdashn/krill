#include "cycle/Cycle.hpp"

#include <catch2/catch.hpp>

using namespace krill;

namespace test
{
  Cycle makeCycle(const Fraction& length, const std::vector<std::pair<Fraction, std::string>>& eventDefinition);
  Cycle simpleCycle(const std::vector<std::string>& values);
  bool compare(const Cycle& c1, const Cycle& c2);
}
