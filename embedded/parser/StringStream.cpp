#include "StringStream.hpp"

#include <string>
#include <cassert>

namespace krill
{
namespace
{
const char* sDelimiters = " \t\n";
//
//std::string trim(const std::string& s)
//{
//	std::string::size_type n, n2;
//	n = s.find_first_not_of(sDelimiters);
//	if (n == std::string::npos)
//		return {};
//	n2 = s.find_last_not_of(sDelimiters);
//	return s.substr(n, n2);
//}
} // namespace

StringStream::StringStream(const std::string& source)
		: mSource(source)
		, mPosition(0)
	{}

bool StringStream::finished()
{
	return mPosition >= mSource.size();
}

bool StringStream::consumeWord(const std::string& expected)
{
	assert(!finished());

	const auto c = current();
	const auto firstNonDelimiter = c.find_first_not_of(sDelimiters);
	if (firstNonDelimiter == std::string::npos)
	{
		return false;
	}

	const auto nextDelimiter = c.find_first_of(sDelimiters);

	const bool endReached = nextDelimiter == std::string::npos;
	const auto wordSize = endReached ? c.size() - firstNonDelimiter : nextDelimiter - firstNonDelimiter;
	const auto word = c.substr(firstNonDelimiter, wordSize);
	if (word != expected)
		return false;

	// Advance position
	const auto nextStart = c.find_first_not_of(sDelimiters, nextDelimiter);
	// Position past the last delimiter if there was one
	const auto offset = nextStart == std::string::npos ? c.size() : nextStart;
	mPosition += offset;
	return true;
}

std::string StringStream::current()
{
	return mSource.substr(mPosition, std::string::npos);
}
} // namespace krill