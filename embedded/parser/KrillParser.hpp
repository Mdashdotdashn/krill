#pragma once

#include "Types.hpp"
#include "third_party/rapidjson/document.h"

#include <memory>
#include <string>

namespace krill {

// New PEG-based parser built on cpp-peglib + kKrillGrammar.
// Semantic actions are added incrementally across Epic 3 stories.
// Intended to replace Parser.cpp (Epic 4) once all actions are wired.
class KrillParser {
public:
    KrillParser();
    ~KrillParser();

    ParsingResult parse(rapidjson::Document& document, const std::string& input);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace krill
