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
    PValue   result; // top-level result, written by the 'start' action
};

// ── Utilities ─────────────────────────────────────────────────────────────────

static std::string trimWs(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, last - first + 1);
}

// Return the first PValue found in vs — used by wrapper rules whose vs may
// also contain spurious entries from whitespace/delimiter rules.
static std::any firstPValue(const peg::SemanticValues& vs)
{
    for (const auto& v : vs)
        if (v.type() == typeid(PValue)) return v;
    return {};
}

// Collect all PValues from vs into a rapidjson array (deep-copies each value).
static rapidjson::Value collectArray(const peg::SemanticValues& vs,
                                     rapidjson::Document::AllocatorType& alloc)
{
    rapidjson::Value arr(rapidjson::kArrayType);
    for (const auto& v : vs) {
        if (v.type() != typeid(PValue)) continue;
        rapidjson::Value copy;
        copy.CopyFrom(*std::any_cast<PValue>(v), alloc);
        arr.PushBack(copy, alloc);
    }
    return arr;
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
        // ── S3.1: step → element ──────────────────────────────────────────────
        parser_["step"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            return std::make_shared<rapidjson::Value>(
                buildXmlForElement(ud.ctx, trimWs(vs.token_to_string())));
        };

        // ── S3.2: slice — pass-through (sub_cycle / timeline / step) ─────────
        parser_["slice"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.2: slice_with_modifier — pass-through (modifier ignored) ───────
        // Modifier support (options_) added in a later story.
        parser_["slice_with_modifier"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.4: single_cycle — horizontal pattern of slice_with_modifier ────
        parser_["single_cycle"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            auto  arr = collectArray(vs, ud.ctx.document().GetAllocator());
            return std::make_shared<rapidjson::Value>(buildXmlForPattern(ud.ctx, arr, "h"));
        };

        // ── S3.4: stack — vertical pattern of single_cycles ──────────────────
        parser_["stack"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            auto  arr = collectArray(vs, ud.ctx.document().GetAllocator());
            return std::make_shared<rapidjson::Value>(buildXmlForPattern(ud.ctx, arr, "v"));
        };

        // ── S3.4: sequence — quoted wrapper, pass through stack result ────────
        parser_["sequence"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.2: sub_cycle — bracketed wrapper, pass through stack result ────
        parser_["sub_cycle"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.3: timeline — change pattern alignment to "t" ─────────────────
        parser_["timeline"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto  pv = std::any_cast<PValue>(firstPValue(vs));
            if (pv && pv->IsObject() && pv->HasMember("arguments_")) {
                (*pv)["arguments_"]["alignment"]
                    .SetString("t", 1, ud.ctx.document().GetAllocator());
            }
            return pv;
        };

        // ── Glue pass-throughs (no transformation, just propagate value) ──────
        auto passThrough = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };
        parser_["sequence_or_group"]    = passThrough;
        parser_["sequence_or_operator"] = passThrough;
        parser_["sequence_definition"]  = passThrough;
        parser_["statement"]            = passThrough;

        // ── start — capture top-level result into UserData ────────────────────
        parser_["start"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto  pv = firstPValue(vs);
            if (pv.has_value()) ud.result = std::any_cast<PValue>(pv);
            return {};
        };

        // S3.5: operator actions (slow, fast, euclid, add, scale, struct, rotL/R, cat, stack_op)
        // S3.6: command actions (hush, setcps, setbpm)
    }

    ParsingResult parse(rapidjson::Document& document, const std::string& input)
    {
        Context  ctx(document, input);
        UserData ud{ctx, nullptr};
        std::any dt = ud;

        const bool ok = parser_.parse(input.c_str(), dt);
        if (!ok) throw ParsingException();

        const auto& captured = std::any_cast<const UserData&>(dt).result;
        if (!captured) return {};
        return std::move(*captured);
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
