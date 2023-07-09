#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <FractionClass/Fraction.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

std::string toString(Fraction& fraction)
{
  std::ostringstream ss;
  ss << fraction;
  return ss.str();
}

TEST_CASE("toString")
{
  Fraction fraction;
  fraction.convertDoubleToFraction(1.5);
  CHECK(toString(fraction) == "3/2");
}


TEST_CASE("rapidjason")
{
  using namespace rapidjson;

  std::ifstream ifs { R"(./test.json)" };
  CHECK(ifs.is_open());

  IStreamWrapper isw { ifs };
  Document document {};
  CHECK(!document.ParseStream(isw).HasParseError());
}
