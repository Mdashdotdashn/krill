#include "parser/Helpers.hpp"
#include "parser/Parser.hpp"
#include "parser/StringStream.hpp"

#include <third_party/catch2/catch.hpp>
#include "third_party/rapidjson/error/en.h"

#include <iostream>

using namespace krill;
using namespace rapidjson;

namespace
{
void checkParsing(const std::string& input, const rapidjson::Value& expected)
{
	try
	{
		Parser parser;
		Document parsingDocument;
		auto result = parser.parse(parsingDocument, input);

		bool similar = (result.value() == expected);
		if (!similar)
		{
			std::cout << "Document differ" << std::endl;
			krill::Dump("Expected", expected);
			krill::Dump("Parsed", result.value());
		}
			
		CHECK(similar);
	}
	catch (const ParsingException& /*e*/)
	{
		CHECK(false);
	}
}

void checkParsing(const std::string& input, const std::string& expected)
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

	checkParsing(input, expectedDocument);
};

void checkParsingFromXml(const std::string xml)
{
    Document d;
		auto doubleQuoted = xml;
		std::replace(doubleQuoted.begin(), doubleQuoted.end(), '\'', '\"');
		d.Parse(doubleQuoted.c_str());
		if (d.HasParseError())
		{
			std::cout << "Error parsing expected string: " << GetParseError_En(d.GetParseError()) << std::endl;
		}
		assert(!d.HasParseError());

		std::string doubleQuote = "\"";
		std::string source = doubleQuote + d["source"].GetString() + doubleQuote;
		checkParsing(source, d["model"]);
}

void expectException(const std::string& s)
{
	try 
	{
  	Parser parser;
		Document d;
		parser.parse(d, s);
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

	SECTION("setcps")
	{
	  checkParsing("setcps 0.34e4"
	  , "{ 'type_': 'command', 'name_': 'setcps', 'options_': {'value': 0.34e4 }}");
	  expectException("setcps not-a-number");
	}

	SECTION("setbpm")
	{
	  checkParsing("setbpm 120"
	  , "{ 'type_': 'command', 'name_': 'setcps', 'options_': {'value': 0.5 }}");
	  expectException("setcps not-a-number");
	}

  SECTION("slice")
  {
    checkParsingFromXml("{ 'source': 'a', 'model': { 'type_': 'element', 'source_': 'a' } }");
		checkParsingFromXml("{ 'source': 'a b', 'model': { 'type_': 'pattern', 'arguments_': { 'alignment': 'h' }, 'source_': [ { 'type_': 'element', 'source_': 'a' }, { 'type_': 'element', 'source_': 'b' } ] }}");
		checkParsingFromXml("{ 'source': 'a b, c','model': {'type_':'pattern','arguments_':{'alignment':'v'},'source_':[{'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'a'},{'type_':'element','source_':'b'}]},{'type_':'element','source_':'c'}]}}");
		checkParsingFromXml("{ 'source': 'a [2,4]','model': {'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'a'},{'type_':'element','source_':{'type_':'pattern','arguments_':{'alignment':'v'},'source_':[{'type_':'element','source_':'2'},{'type_':'element','source_':'4'}]}}]}}");
	}

  SECTION("timeline (S5.2 — previously assert(0) in MiniNotation)")
  {
    // <a b> → element wrapping a pattern with alignment "t"
    checkParsingFromXml("{ 'source': '<a b>', 'model': {'type_':'element','source_':{'type_':'pattern','arguments_':{'alignment':'t'},'source_':[{'type_':'element','source_':'a'},{'type_':'element','source_':'b'}]}}}");
    // single-element timeline passes through as element
    checkParsingFromXml("{ 'source': '<a>', 'model': {'type_':'element','source_':'a'}}");
    // timeline inside a sequence — timeline slot becomes element wrapping pattern "t"
    checkParsingFromXml("{ 'source': 'x <a b> y', 'model': {'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'x'},{'type_':'element','source_':{'type_':'pattern','arguments_':{'alignment':'t'},'source_':[{'type_':'element','source_':'a'},{'type_':'element','source_':'b'}]}},{'type_':'element','source_':'y'}]}}");
  }

  SECTION("operators (S5.4 — full round-trip smoke)")
  {
    // slow
    checkParsing("slow 2 $ 'a b'", "{'type_':'stretch','arguments_':[2.0],'source_':{'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'a'},{'type_':'element','source_':'b'}]}}");
		// fast
		checkParsing("fast 4 $ 'a b'", "{'type_':'stretch','arguments_':['1/4'],'source_':{'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'a'},{'type_':'element','source_':'b'}]}}");
    // euclid
    checkParsing("euclid 5 8 $ 'bd'", "{'type_':'bjorklund','arguments_':[5,8],'source_':{'type_':'element','source_':'bd'}}");
    // chained: slow $ euclid $ sequence
    checkParsing("slow 2 $ euclid 5 8 $ 'bd'", "{'type_':'stretch','arguments_':[2.0],'source_':{'type_':'bjorklund','arguments_':[5,8],'source_':{'type_':'element','source_':'bd'}}}");
  }

	SECTION("rotation operator canonicalization")
	{
		checkParsing("rotR 0.125 $ 'bd ~ sd ~'", "{'type_':'shift','arguments_':['0.125',1],'source_':{'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'bd'},{'type_':'element','source_':'~'},{'type_':'element','source_':'sd'},{'type_':'element','source_':'~'}]}}");
		checkParsing("rotL '<0 0.125>' $ 'bd ~ sd ~'", "{'type_':'shift','arguments_':[{'type_':'element','source_':{'type_':'pattern','arguments_':{'alignment':'t'},'source_':[{'type_':'element','source_':'0'},{'type_':'element','source_':'0.125'}]}},-1],'source_':{'type_':'pattern','arguments_':{'alignment':'h'},'source_':[{'type_':'element','source_':'bd'},{'type_':'element','source_':'~'},{'type_':'element','source_':'sd'},{'type_':'element','source_':'~'}]}}");
	}

	SECTION("timing transform canonicalization")
	{
		checkParsingFromXml("{ 'source': '[a]/2', 'model': { 'type_':'element', 'source_': {'type_':'element','source_':'a'}, 'options_': {'operator': {'type_':'stretch','arguments_':[2.0]}} } }");
		checkParsingFromXml("{ 'source': '[a]*4', 'model': { 'type_':'element', 'source_': {'type_':'element','source_':'a'}, 'options_': {'operator': {'type_':'stretch','arguments_':['1/4']}} } }");
		checkParsingFromXml("{ 'source': '[a]%3', 'model': { 'type_':'element', 'source_': {'type_':'element','source_':'a'}, 'options_': {'operator': {'type_':'fixed-step','arguments_':[3.0]}} } }");
	}
}
