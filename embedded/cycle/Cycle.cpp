#include "Cycle.hpp"


namespace krill
{
  Cycle makeEmptyCycle()
  {
    return Cycle{};
  }

   // make a cycle out of a single event
  Cycle makeSingleEventCycle (std::string value)
  {
    std::vector<Cycle::Event> eventArray;
    eventArray.push_back({0,{value}});
    return Cycle{eventArray};
  }  
}
