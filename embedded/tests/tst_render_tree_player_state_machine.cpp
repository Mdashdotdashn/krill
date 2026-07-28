#include "renderer/RenderTreeBuilder.hpp"
#include "renderer/RenderTreePlayer.hpp"
#include "parser/Parser.hpp"

#include <third_party/catch2/catch.hpp>
#include <third_party/rapidjson/document.h>

#include <string>
#include <vector>

namespace
{
krill::RenderNodePtr makeTree(const std::string& source)
{
  krill::Parser parser;
  rapidjson::Document parseDoc;
  auto parseResult = parser.parse(parseDoc, source);
  REQUIRE(parseResult.has_value());
  return krill::RenderTreeBuilder::fromJson(parseResult.value());
}

std::string fracToString(Fraction value)
{
  return static_cast<std::string>(value);
}
} // namespace

TEST_CASE("RenderTreePlayer advance monotonicity and onset extraction")
{
  krill::RenderTreePlayer player;
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  const auto t0 = player.nextOnsetTimeFrom(Fraction("-1/10000"));
  CHECK(t0 == Fraction("0/1"));

  const auto e0 = player.eventsAtTime(t0);
  REQUIRE(!e0.empty());
  CHECK(e0 == std::vector<std::string>{"a"});
  CHECK(player.nextOnsetTimeFrom(Fraction("0/1")) == Fraction("1/4"));

  const auto e1 = player.eventsAtTime(Fraction("1/4"));
  REQUIRE(!e1.empty());
  CHECK(e1 == std::vector<std::string>{"b"});
  CHECK(player.nextOnsetTimeFrom(Fraction("1/4")) == Fraction("1/2"));

  const auto e2 = player.eventsAtTime(Fraction("1/2"));
  REQUIRE(!e2.empty());
  CHECK(e2 == std::vector<std::string>{"c"});
  CHECK(player.nextOnsetTimeFrom(Fraction("1/2")) == Fraction("3/4"));

  const auto e3 = player.eventsAtTime(Fraction("3/4"));
  REQUIRE(!e3.empty());
  CHECK(e3 == std::vector<std::string>{"d"});
  CHECK(player.nextOnsetTimeFrom(Fraction("3/4")) == Fraction("1/1"));
}

TEST_CASE("RenderTreePlayer tree replacement at cycle boundary")
{
  krill::RenderTreePlayer player;
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  const auto atHalf = player.eventsAtTime(Fraction("1/2"));
  REQUIRE(!atHalf.empty());
  CHECK(atHalf == std::vector<std::string>{"c"});

  player.setTree(makeTree("'x y'"));

  const auto atThreeQuarters = player.eventsAtTime(Fraction("3/4"));
  REQUIRE(!atThreeQuarters.empty());
  CHECK(atThreeQuarters == std::vector<std::string>{"d"});

  const auto nextAfterThreeQuarters = player.nextOnsetTimeFrom(Fraction("3/4"));
  CAPTURE(fracToString(nextAfterThreeQuarters));
  REQUIRE(nextAfterThreeQuarters == Fraction("1/1"));

  const auto atOne = player.eventsAtTime(Fraction("1/1"));
  REQUIRE(!atOne.empty());
  CHECK(atOne == std::vector<std::string>{"x"});

  const auto atThreeHalves = player.eventsAtTime(Fraction("3/2"));
  REQUIRE(!atThreeHalves.empty());
  CHECK(atThreeHalves == std::vector<std::string>{"y"});
}

