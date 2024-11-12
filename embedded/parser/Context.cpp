#include "Context.hpp"

namespace krill
{
Context::Context(rapidjson::Document& document, const std::string& source)
	: mDocument(document)
	, mStream(source)
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

std::optional<std::string> Context::consumeDelimitedString(const std::string & delimiters)
{
  const std::string expression = " *["+delimiters+"]([^"+delimiters+"]*)["+delimiters+"]";
	const auto result = mStream.consumeExpression(expression);
  if (result.size() > 0)
  {
    assert(result.size() == 1);
    return result[0];
  }
  return {};
}

std::vector<std::string> Context::consumeTokens(const std::string & separators)
{
  return mStream.consumeTokens(separators);
}

rapidjson::Document& Context::document()
{
  return mDocument;
}

Context Context::buildSubContext(std::string& subString)
{
	return {mDocument, subString};
}
} // namespace krill