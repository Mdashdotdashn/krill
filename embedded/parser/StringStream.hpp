#pragma once

#include <optional>
#include <string>
#include <vector>

namespace krill
{
class ParsingException: public std::exception
{
public:
	ParsingException(): std::exception()
	{
	}
};

class StringStream
{
public:
	StringStream(const std::string& source);

	bool finished();

	bool consumeWord(const std::string& word);
	std::optional<std::string> consumeFloat();
  std::vector<std::string> consumeExpression(const std::string& expression);
	std::vector<std::string> consumeTokens(const std::string& separator);

private:
	std::string current();
	void advance(size_t offset);

	const std::string mSource;
	size_t mPosition;
};
} // namespace krill