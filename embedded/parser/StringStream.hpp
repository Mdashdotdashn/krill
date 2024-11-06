#pragma once

#include <string>

namespace krill
{
class StringStream
{
public:
	StringStream(const std::string& source);

	bool finished();

	bool consumeWord(const std::string& word);

private:
	std::string current();

	const std::string mSource;
	size_t mPosition;
};
} // namespace krill