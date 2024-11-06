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
}