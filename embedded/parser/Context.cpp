#include "Context.hpp"

namespace krill
{
Context::Context(rapidjson::Document& document, const std::string& source)
	:mDocument(document)
	,mStream(source)
{
	mDocument.SetObject();
}

bool Context::consumeToken(const std::string& token)
{
	return mStream.consumeWord(token);
}

std::optional<std::string> Context::consumeFloat()
{
	return mStream.consumeFloat();
}

void Context::addCommand(const std::string& command, const std::optional<std::string> value)
{
	using namespace rapidjson;

	// Commands are top level so we can add directly to the document
	mDocument.AddMember("type_", "command", mDocument.GetAllocator());
	Value name_;
	name_.SetString(command.c_str(), rapidjson::SizeType(command.size()), mDocument.GetAllocator());
	mDocument.AddMember("name_", name_, mDocument.GetAllocator());
	if (value)
	{
		const float floatValue = std::stof(value.value());
		Value value_;
		value_.SetFloat(floatValue);
		Value options_(kObjectType);
		options_.AddMember("value", value_, mDocument.GetAllocator());
		mDocument.AddMember("options_", options_, mDocument.GetAllocator());
	}
}
} // namespace krill