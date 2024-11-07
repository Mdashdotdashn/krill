#pragma once

#include <optional>
#include <string>

namespace krill
{
using ParsingException = std::exception;

class StringStream
{
public:
	StringStream(const std::string& source);

	bool finished();

	bool consumeWord(const std::string& word);
	std::optional<std::string> consumeFloat();

private:
	std::string current();

	const std::string mSource;
	size_t mPosition;
};
} // namespace krill