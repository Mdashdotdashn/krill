#include "StringStream.hpp"

#include <iostream>
#include <string>
#include <sstream>
#include <cassert>
#include <regex>

namespace krill
{
namespace
{
const std::string sWordDelimiters(" \t\n");
const std::string sAnyDelimitersExpr("["+sWordDelimiters+"]*");

std::string trim(const std::string& s)
{
	std::string::size_type n, n2;
	n = s.find_first_not_of(sWordDelimiters);
	if (n == std::string::npos)
		return {};
	n2 = s.find_last_not_of(sWordDelimiters);
	return s.substr(n, n2 - n + 1);
}
} // namespace

StringStream::StringStream(const std::string& source)
		: mSource(source)
		, mPosition(0)
	{}

bool StringStream::finished()
{
	return mPosition >= mSource.size();
}

std::vector<std::string> StringStream::consumeExpression(const std::string& expression)
{
  const auto c = current();
  std::regex pattern(expression);
  std::smatch match;

  std::vector<std::string> result;

  if (std::regex_search(c, match, pattern))
  {
		const auto offset = match.position() + match.length();
		advance(offset);

		// Capture groups used
		if (match.size() > 1)
		{
			for (size_t index = 1; index < match.size(); index++)
			{
				result.push_back(match.str(index));
			}
		}
		else
		// No groups, return the full match
		{
			result.push_back(match.str());
		}
	}
  return result;
}

std::vector<std::string> StringStream::consumeTokens(const std::string& separator) {

		const auto c = current();
    std::vector<std::string> segments;
    std::regex segmentRegex("[^"+separator+"]+");  // Regex to match any sequence of characters except commas
    std::sregex_token_iterator iter(c.begin(), c.end(), segmentRegex);
    std::sregex_token_iterator end;

    // Collect each match in the vector
    while (iter != end) {
        segments.push_back(*iter);
        ++iter;
    }

		// we consume everything
		mPosition = mSource.size();
    return segments;
}
bool StringStream::consumeWord(const std::string& expected)
{
	assert(!finished());

	const auto c = current();
	const auto expression = sAnyDelimitersExpr+expected+sAnyDelimitersExpr;;
	return consumeExpression(expression).size() == 1;
}

std::optional<std::string> StringStream::consumeFloat()
{
	float f;
	const auto c = current();
	std::istringstream iss(c);
	if (iss >> f)
	{
		size_t offset = iss.tellg();
		mPosition+=offset;
		return trim(c.substr(0, offset));
	}
	return {};
}

std::string StringStream::current()
{
	const auto current = mSource.substr(mPosition, std::string::npos);
//	std::cout << "Current: " << current << std::endl;
	return current;
}

void StringStream::advance(size_t offset)
{
	mPosition += offset;
	mPosition = mSource.find_first_not_of(sWordDelimiters, mPosition);
}
} // namespace krill