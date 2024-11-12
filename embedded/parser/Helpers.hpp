#pragma once

#include "third_party/rapidjson/document.h"
#include "third_party/rapidjson/stringbuffer.h"
#include "third_party/rapidjson/writer.h"

#include <iostream>
#include <string>

namespace krill
{
static void Dump(const std::string& label, const rapidjson::Value& value)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	value.Accept(writer);
	std::cout << "Parsing result: " << label << std::endl << " " << buffer.GetString() << std::endl;
}
}