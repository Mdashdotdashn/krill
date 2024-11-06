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

void Context::addCommand(const std::string& command)
{
	using namespace rapidjson;

	// Commands are top level so we can add directly to the document
	mDocument.AddMember("type_", "command", mDocument.GetAllocator());
	Value v;
	v.SetString(command.c_str(), rapidjson::SizeType(command.size()), mDocument.GetAllocator());
	mDocument.AddMember("name_", v, mDocument.GetAllocator());
}
} // namespace krill