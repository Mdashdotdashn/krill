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

  const auto t0 = player.advance(Fraction("-1/10000"));
  CHECK(t0 == Fraction("0/1"));

  const auto e0 = player.eventForTime(t0);
  REQUIRE(e0.has_value());
  CHECK(e0->values == std::vector<std::string>{"a"});
  CHECK(player.advance(Fraction("0/1")) == Fraction("1/4"));

  const auto e1 = player.eventForTime(Fraction("1/4"));
  REQUIRE(e1.has_value());
  CHECK(e1->values == std::vector<std::string>{"b"});
  CHECK(player.advance(Fraction("1/4")) == Fraction("1/2"));

  const auto e2 = player.eventForTime(Fraction("1/2"));
  REQUIRE(e2.has_value());
  CHECK(e2->values == std::vector<std::string>{"c"});
  CHECK(player.advance(Fraction("1/2")) == Fraction("3/4"));

  const auto e3 = player.eventForTime(Fraction("3/4"));
  REQUIRE(e3.has_value());
  CHECK(e3->values == std::vector<std::string>{"d"});
  CHECK(player.advance(Fraction("3/4")) == Fraction("1/1"));
}

TEST_CASE("RenderTreePlayer tree replacement at cycle boundary")
{
  krill::RenderTreePlayer player;
  player.setTree(makeTree("'a b c d'"));
  player.reset();

  const auto atHalf = player.eventForTime(Fraction("1/2"));
  REQUIRE(atHalf.has_value());
  CHECK(atHalf->values == std::vector<std::string>{"c"});

  player.setTree(makeTree("'x y'"));

  const auto atThreeQuarters = player.eventForTime(Fraction("3/4"));
  REQUIRE(atThreeQuarters.has_value());
  CHECK(atThreeQuarters->values == std::vector<std::string>{"d"});

  const auto nextAfterThreeQuarters = player.advance(Fraction("3/4"));
  CAPTURE(fracToString(nextAfterThreeQuarters));
  REQUIRE(nextAfterThreeQuarters == Fraction("1/1"));

  const auto atOne = player.eventForTime(Fraction("1/1"));
  REQUIRE(atOne.has_value());
  CHECK(atOne->values == std::vector<std::string>{"x"});

  const auto atThreeHalves = player.eventForTime(Fraction("3/2"));
  REQUIRE(atThreeHalves.has_value());
  CHECK(atThreeHalves->values == std::vector<std::string>{"y"});
}

TEST_CASE("RenderTreePlayer eventsForTime alias")
{
  krill::RenderTreePlayer player;
  player.setTree(makeTree("'bd sd'"));
  player.reset();

  CHECK(player.eventsForTime(Fraction("0/1")) == std::vector<std::string>{"bd"});
  CHECK(player.eventsForTime(Fraction("1/2")) == std::vector<std::string>{"sd"});
  CHECK(player.eventsForTime(Fraction("1/4")).empty());
}