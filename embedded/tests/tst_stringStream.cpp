#include "parser/StringStream.hpp"

#include <third_party/catch2/catch.hpp>

using namespace krill;

TEST_CASE("StringStream")
{
	SECTION("consumeWord")
	{
		StringStream s("one two   three\n");
		CHECK(!s.consumeWord("ones"));
		CHECK(s.consumeWord("one"));
		CHECK(s.consumeWord("two"));
		CHECK(s.consumeWord("three"));
		CHECK(s.finished());
	}

	SECTION("consumeFloat")
	{
		StringStream s("1.3  -7.5 a   40e-5");
		CHECK(s.consumeFloat() == "1.3");
		CHECK(s.consumeFloat() == "-7.5");
		CHECK(!s.consumeFloat());
		CHECK(s.consumeWord("a"));
		CHECK(s.consumeFloat() == "40e-5");
	}
}