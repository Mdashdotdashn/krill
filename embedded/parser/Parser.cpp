#include "Parser.hpp"

namespace krill {

ParsingResult Parser::parse(rapidjson::Document& document, const std::string& input)
{
    return kp_.parse(document, input);
}

} // namespace krill
