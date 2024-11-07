#include "parser/Parser.hpp"
#include "parser/StringStream.hpp"

#include <third_party/catch2/catch.hpp>
#include "third_party/rapidjson/error/en.h"
#include "third_party/rapidjson/writer.h"
#include "third_party/rapidjson/stringbuffer.h"

#include <iostream>

using namespace krill;
using namespace rapidjson;

namespace
{
void Dump(const std::string& label, const Document& document)
{
	StringBuffer buffer;
	Writer<StringBuffer> writer(buffer);
	document.Accept(writer);
	std::cout << "Parsing result: " << label << std::endl << buffer.GetString() << std::endl;
}
void checkParsing(const std::string& input, const std::string& expected)
{
	try
	{
		Document expectedDocument;
		auto doubleQuoted = expected;
		std::replace(doubleQuoted.begin(), doubleQuoted.end(), '\'', '\"');
		expectedDocument.Parse(doubleQuoted.c_str());
		if (expectedDocument.HasParseError())
		{
			std::cout << "Error parsing expected string: " << GetParseError_En(expectedDocument.GetParseError()) << std::endl;
		}
		assert(!expectedDocument.HasParseError());

		Parser parser;
		rapidjson::Document parsedDocument;
		parser.parse(parsedDocument, input);

		bool similar = (parsedDocument == expectedDocument);
		if (!similar)
		{
			std::cout << "Document differ" << std::endl;
			Dump("Expected", expectedDocument);
			Dump("Parsed", parsedDocument);
		}
			
		CHECK(similar);
	}
	catch (const ParsingException& /*e*/)
	{
		CHECK(false);
	}
};

void expectException(const std::string& s)
{
	Parser parser;
	rapidjson::Document document;

	try 
	{
		parser.parse(document, s);
		CHECK(false);
	}
	catch (const ParsingException& /*e*/)
	{
	}
};
} // namespace

TEST_CASE("Parser")
{
	SECTION("Garbage throws exception")
	{
	  expectException("garbage");
	}

	SECTION("hush")
	{
	  checkParsing("hush", "{'type_':'command','name_':'hush'}");
	}

	SECTION("setCps")
	{
	  checkParsing("setcps 0.34e4"
	  , "{ 'type_': 'command', 'name_': 'setcps', 'options_': {'value': 0.34e4 }}");
	  expectException("setcps not-a-number");
	}
}
