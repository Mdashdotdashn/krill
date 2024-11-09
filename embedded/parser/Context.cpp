#include "Context.hpp"

namespace krill
{
Context::Context(const std::string& source)
	: mStream(source)
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

rapidjson::Document& Context::document()
{
  return mDocument;
}
} // namespace krill