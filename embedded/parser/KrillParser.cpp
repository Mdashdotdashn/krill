#include "KrillParser.hpp"
#include "KrillGrammar.hpp"
#include "Context.hpp"
#include "XmlBuilder.hpp"

#include <peglib.h>
#include <cassert>
#include <memory>
#include <string>

namespace krill {

// ── Shared types ──────────────────────────────────────────────────────────────

// rapidjson::Value is move-only — it cannot be stored in std::any directly.
// PValue (shared_ptr) is copy-constructible and works transparently with std::any.
using PValue = std::shared_ptr<rapidjson::Value>;

// Per-parse user data passed to all semantic actions via std::any& dt.
struct UserData {
    Context& ctx;
};

// ── Utilities ─────────────────────────────────────────────────────────────────

static std::string trimWs(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

// ── Impl ──────────────────────────────────────────────────────────────────────

struct KrillParser::Impl {
    peg::parser parser_;

    Impl() : parser_(kKrillGrammar)
    {
        assert(parser_); // grammar must compile
        setupActions();
    }

    void setupActions()
    {
        // ── S3.1: step → buildXmlForElement ──────────────────────────────────
        parser_["step"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            const auto token = trimWs(vs.token_to_string());
            return std::make_shared<rapidjson::Value>(buildXmlForElement(ud.ctx, token));
        };

        // Further actions added in S3.2 – S3.6.
    }

    ParsingResult parse(rapidjson::Document& document, const std::string& input)
    {
        Context ctx(document, input);
        UserData ud{ctx};
        std::any dt = ud; // UserData is copied; its Context& still refers to ctx above

        // Full result capture requires all Epic 3 stories. Returns empty for now.
        parser_.parse(input.c_str(), dt);
        return {};
    }
};

// ── KrillParser ───────────────────────────────────────────────────────────────

KrillParser::KrillParser()  : impl_(std::make_unique<Impl>()) {}
KrillParser::~KrillParser() = default;

ParsingResult KrillParser::parse(rapidjson::Document& document, const std::string& input)
{
    return impl_->parse(document, input);
}

} // namespace krill
