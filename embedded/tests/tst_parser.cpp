#include "parser/Parser.hpp"

#include <third_party/catch2/catch.hpp>
#include "third_party/rapidjson/writer.h"
#include "third_party/rapidjson/stringbuffer.h"

#include <iostream>

using namespace krill;
using namespace rapidjson;

TEST_CASE("Parser")
{
	SECTION("Garbage throws exception")
	{
		Parser parser;
		rapidjson::Document document;

		try
		{
			parser.parse(document, "garbage");
			// Should not get here
			CHECK(false);
		}
		catch (const ParsingException& /*e*/)
		{
		}
	}

	const auto parseWithNoError = [&](const std::string& s)
	{
		Parser parser;
		rapidjson::Document document;

		try
		{
			parser.parse(document, s);
			StringBuffer buffer;
			Writer<StringBuffer> writer(buffer);
			document.Accept(writer);
#if 1
			std::cout << "Parsing result: " << buffer.GetString() << std::endl;
#endif
			return buffer.GetString();
		}
		catch (const ParsingException& /*e*/)
		{
			CHECK(false);
		}
	};

	SECTION("hush")
	{
	  parseWithNoError("hush");
	}
}