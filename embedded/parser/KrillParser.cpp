#include "KrillParser.hpp"
#include "KrillGrammar.hpp"
#include "Context.hpp"
#include "XmlBuilder.hpp"

#include <peglib.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
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

// Intermediate type used by operator rules to carry name + args to sequence_or_operator.
// Stored as std::any so it can travel through SemanticValues alongside PValues.
struct OperatorInfo {
    std::string name;
    PValue      args; // shared_ptr to a JSON array
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

static std::any firstOperatorArgumentValue(const peg::SemanticValues& vs)
{
    for (const auto& v : vs)
    {
        if (v.type() == typeid(PValue)
            || v.type() == typeid(std::string)
            || v.type() == typeid(double)
            || v.type() == typeid(int))
        {
            return v;
        }
    }
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

static void pushAnyToJsonArray(rapidjson::Value& arr,
                               const std::any& value,
                               rapidjson::Document::AllocatorType& alloc)
{
    if (value.type() == typeid(PValue))
    {
        rapidjson::Value copy;
        copy.CopyFrom(*std::any_cast<PValue>(value), alloc);
        arr.PushBack(copy, alloc);
        return;
    }

    if (value.type() == typeid(std::string))
    {
        const auto& s = std::any_cast<const std::string&>(value);
        rapidjson::Value v;
        v.SetString(s.c_str(), rapidjson::SizeType(s.size()), alloc);
        arr.PushBack(v, alloc);
        return;
    }

    if (value.type() == typeid(double))
    {
        arr.PushBack(rapidjson::Value(std::any_cast<double>(value)), alloc);
        return;
    }

    if (value.type() == typeid(int))
    {
        arr.PushBack(rapidjson::Value(std::any_cast<int>(value)), alloc);
    }
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
        // ── S3.1: step → raw step string ──────────────────────────────────────
        parser_["step"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            rapidjson::Value step;
            const auto source = trimWs(vs.token_to_string());
            step.SetString(source.c_str(), rapidjson::SizeType(source.size()), alloc);
            return std::make_shared<rapidjson::Value>(std::move(step));
        };

        // ── S3.2: slice — pass-through (sub_cycle / timeline / step) ─────────
        parser_["slice"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.2: slice modifiers → options_ objects ──────────────────────────

        // slice_weight '@' number → {weight: n}
        parser_["slice_weight"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            rapidjson::Value opts(rapidjson::kObjectType);
            for (const auto& v : vs)
                if (v.type() == typeid(double)) {
                    opts.AddMember("weight", rapidjson::Value(std::any_cast<double>(v)), alloc);
                    break;
                }
            return std::make_shared<rapidjson::Value>(std::move(opts));
        };

        // slice_slow '/' number → {operator:{type_:"stretch",arguments_:[n]}}
        parser_["slice_slow"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            rapidjson::Value args(rapidjson::kArrayType);
            for (const auto& v : vs)
                if (v.type() == typeid(double)) {
                    args.PushBack(rapidjson::Value(std::any_cast<double>(v)), alloc); break;
                }
            rapidjson::Value op(rapidjson::kObjectType);
            op.AddMember("type_", rapidjson::StringRef("stretch"), alloc);
            op.AddMember("arguments_", args, alloc);
            rapidjson::Value opts(rapidjson::kObjectType);
            opts.AddMember("operator", op, alloc);
            return std::make_shared<rapidjson::Value>(std::move(opts));
        };

        // slice_fast '*' number → {operator:{type_:"stretch",arguments_:["1/n"]}}
        parser_["slice_fast"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            double n = 1.0;
            for (const auto& v : vs)
                if (v.type() == typeid(double)) { n = std::any_cast<double>(v); break; }
            const auto frac = "1/" + std::to_string(static_cast<int>(n));
            rapidjson::Value fracVal;
            fracVal.SetString(frac.c_str(), rapidjson::SizeType(frac.size()), alloc);
            rapidjson::Value args(rapidjson::kArrayType);
            args.PushBack(fracVal, alloc);
            rapidjson::Value op(rapidjson::kObjectType);
            op.AddMember("type_", rapidjson::StringRef("stretch"), alloc);
            op.AddMember("arguments_", args, alloc);
            rapidjson::Value opts(rapidjson::kObjectType);
            opts.AddMember("operator", op, alloc);
            return std::make_shared<rapidjson::Value>(std::move(opts));
        };

        // slice_bjorklund '(' number ',' number ')' → {operator:{type_:"bjorklund",arguments_:[p,s]}}
        parser_["slice_bjorklund"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();

            std::vector<int> values;
            values.reserve(2);
            for (const auto& v : vs)
            {
                if (v.type() == typeid(double))
                {
                    values.push_back(static_cast<int>(std::any_cast<double>(v)));
                }
            }

            rapidjson::Value args(rapidjson::kArrayType);
            if (!values.empty())
            {
                args.PushBack(rapidjson::Value(values[0]), alloc);
            }
            if (values.size() > 1)
            {
                args.PushBack(rapidjson::Value(values[1]), alloc);
            }

            rapidjson::Value op(rapidjson::kObjectType);
            op.AddMember("type_", rapidjson::StringRef("bjorklund"), alloc);
            op.AddMember("arguments_", args, alloc);

            rapidjson::Value opts(rapidjson::kObjectType);
            opts.AddMember("operator", op, alloc);
            return std::make_shared<rapidjson::Value>(std::move(opts));
        };

        // slice_fixed_step '%' number → {operator:{type_:"fixed-step",arguments_:[n]}}
        parser_["slice_fixed_step"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            rapidjson::Value args(rapidjson::kArrayType);
            for (const auto& v : vs)
                if (v.type() == typeid(double)) {
                    args.PushBack(rapidjson::Value(std::any_cast<double>(v)), alloc); break;
                }
            rapidjson::Value typeStr;
            typeStr.SetString("fixed-step", 10, alloc);
            rapidjson::Value op(rapidjson::kObjectType);
            op.AddMember("type_", typeStr, alloc);
            op.AddMember("arguments_", args, alloc);
            rapidjson::Value opts(rapidjson::kObjectType);
            opts.AddMember("operator", op, alloc);
            return std::make_shared<rapidjson::Value>(std::move(opts));
        };

        // slice_modifier: pass the matched sub-rule's PValue up
        parser_["slice_modifier"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.2: slice_with_modifier ─────────────────────────────────────────
        // JS parity: always build ElementStub(source=slice, options_=modifier?)
        parser_["slice_with_modifier"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();

            // Collect PValues in order: slice first, then optional modifier
            std::vector<PValue> pvs;
            for (const auto& v : vs)
                if (v.type() == typeid(PValue)) pvs.push_back(std::any_cast<PValue>(v));

            if (pvs.empty()) return std::any{};

            auto slicePv = pvs[0];
            PValue modPv = pvs.size() > 1 ? pvs[1] : nullptr;

            rapidjson::Value elem(rapidjson::kObjectType);
            elem.AddMember("type_", rapidjson::StringRef("element"), alloc);

            rapidjson::Value srcCopy;
            srcCopy.CopyFrom(*slicePv, alloc);
            elem.AddMember("source_", srcCopy, alloc);

            if (modPv)
            {
                rapidjson::Value optCopy;
                optCopy.CopyFrom(*modPv, alloc);
                elem.AddMember("options_", optCopy, alloc);
            }

            return std::make_shared<rapidjson::Value>(std::move(elem));
        };

        // ── S3.4: single_cycle — horizontal pattern of slice_with_modifier ────
        // JS parity: if only one element, pass-through the element.
        parser_["single_cycle"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            auto  arr = collectArray(vs, ud.ctx.document().GetAllocator());

            if (arr.IsArray() && arr.Size() == 1 && arr[0].IsObject())
            {
                rapidjson::Value copy;
                copy.CopyFrom(arr[0], ud.ctx.document().GetAllocator());
                return std::make_shared<rapidjson::Value>(std::move(copy));
            }

            return std::make_shared<rapidjson::Value>(buildXmlForPattern(ud.ctx, arr, "h"));
        };

        // ── S3.4: stack — vertical pattern of single_cycles ──────────────────
        // JS parity: if only one child and object, pass-through directly.
        parser_["stack"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            auto  arr = collectArray(vs, ud.ctx.document().GetAllocator());

            if (arr.IsArray() && arr.Size() == 1 && arr[0].IsObject())
            {
                rapidjson::Value copy;
                copy.CopyFrom(arr[0], ud.ctx.document().GetAllocator());
                return std::make_shared<rapidjson::Value>(std::move(copy));
            }

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
            if (pv && pv->IsObject()
                && pv->HasMember("type_")
                && (*pv)["type_"].IsString()
                && std::string((*pv)["type_"].GetString()) == "element"
                && pv->HasMember("source_")
                && (*pv)["source_"].IsString())
            {
                rapidjson::Value unwrapped;
                unwrapped.CopyFrom((*pv)["source_"], ud.ctx.document().GetAllocator());
                return std::make_shared<rapidjson::Value>(std::move(unwrapped));
            }

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

        // S3.5: operator actions ───────────────────────────────────────────────

        // number → double (needed by all operators that take a numeric argument)
        parser_["number"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return std::stod(vs.token_to_string());
        };
        // int → int (needed by bjorklund which uses int ws int)
        parser_["int"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return std::stoi(vs.token_to_string());
        };

        // Helper: build a single-float args array from the first double in vs
        auto singleNumArgs = [](const peg::SemanticValues& vs,
                                rapidjson::Document::AllocatorType& alloc,
                                double factor = 1.0) {
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            for (const auto& v : vs)
                if (v.type() == typeid(double)) {
                    args->PushBack(rapidjson::Value(std::any_cast<double>(v) * factor), alloc);
                    break;
                }
            return args;
        };

        parser_["slow"] = [singleNumArgs](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            return OperatorInfo{"stretch", singleNumArgs(vs, ud.ctx.document().GetAllocator())};
        };
        parser_["fast"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            // JS parity: fast N -> stretch ["1/N"]
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            for (const auto& v : vs)
            {
                if (v.type() == typeid(double))
                {
                    const auto inv = 1.0 / std::any_cast<double>(v);
                    const auto invString = std::to_string(inv);
                    char* end = nullptr;
                    const auto asDouble = std::strtod(invString.c_str(), &end);

                    // Prefer exact rational string form when representable as 1/n
                    const auto n = std::any_cast<double>(v);
                    if (n != 0.0)
                    {
                        const auto asInt = static_cast<int>(n);
                        if (std::abs(n - asInt) < 1e-9)
                        {
                            const auto frac = std::string("1/") + std::to_string(asInt);
                            rapidjson::Value fracVal;
                            fracVal.SetString(frac.c_str(), rapidjson::SizeType(frac.size()), alloc);
                            args->PushBack(fracVal, alloc);
                            break;
                        }
                    }

                    rapidjson::Value numeric(asDouble);
                    args->PushBack(numeric, alloc);
                    break;
                }
            }
            return OperatorInfo{"stretch", args};
        };
        parser_["trunc"] = [singleNumArgs](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            return OperatorInfo{"trunc", singleNumArgs(vs, ud.ctx.document().GetAllocator())};
        };
        parser_["rotR"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            const auto arg = firstOperatorArgumentValue(vs);
            if (arg.has_value())
            {
                pushAnyToJsonArray(*args, arg, alloc);
            }
            args->PushBack(rapidjson::Value(1), alloc);
            return OperatorInfo{"shift", args};
        };
        parser_["rotL"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            const auto arg = firstOperatorArgumentValue(vs);
            if (arg.has_value())
            {
                pushAnyToJsonArray(*args, arg, alloc);
            }
            args->PushBack(rapidjson::Value(-1), alloc);
            return OperatorInfo{"shift", args};
        };
        parser_["bjorklund"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud    = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            for (const auto& v : vs)
                if (v.type() == typeid(int))
                    args->PushBack(rapidjson::Value(std::any_cast<int>(v)), alloc);
            return OperatorInfo{"bjorklund", args};
        };
        parser_["scale"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud    = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            const auto token  = vs.token_to_string();
            const auto qstart = token.find_first_of("\"'");
            const auto qend   = token.find_last_of("\"'");
            const auto name   = (qstart < qend) ? token.substr(qstart+1, qend-qstart-1) : "";
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            rapidjson::Value nameVal;
            nameVal.SetString(name.c_str(), rapidjson::SizeType(name.size()), alloc);
            args->PushBack(nameVal, alloc);
            return OperatorInfo{"scale", args};
        };
        parser_["add"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud    = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            const auto arg = firstOperatorArgumentValue(vs);
            if (arg.has_value()) {
                pushAnyToJsonArray(*args, arg, alloc);
            }
            return OperatorInfo{"add", args};
        };
        parser_["struct"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud    = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            auto args = std::make_shared<rapidjson::Value>(rapidjson::kArrayType);
            if (auto pv = std::any_cast<PValue>(firstPValue(vs))) {
                rapidjson::Value copy; copy.CopyFrom(*pv, alloc);
                args->PushBack(copy, alloc);
            }
            return OperatorInfo{"struct", args};
        };

        // operator: pass the OperatorInfo from the matched sub-rule up to sequence_or_operator
        parser_["operator"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return vs.empty() ? std::any{} : vs[0];
        };
        // operator_argument: step or sequence_or_operator → PValue
        parser_["operator_argument"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstOperatorArgumentValue(vs);
        };

        // sequence_or_operator: two alternatives distinguished by vs.choice()
        parser_["sequence_or_operator"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            if (vs.choice() == 0)
                return firstPValue(vs); // sequence_or_group (pass through)

            // operator ws '$' ws sequence_or_operator → build operator node
            auto& ud    = std::any_cast<UserData&>(dt);
            auto& alloc = ud.ctx.document().GetAllocator();
            OperatorInfo opInfo;
            PValue source;
            for (const auto& v : vs) {
                if (v.type() == typeid(OperatorInfo)) opInfo = std::any_cast<OperatorInfo>(v);
                else if (v.type() == typeid(PValue))  source = std::any_cast<PValue>(v);
            }
            if (!opInfo.args || !source) return firstPValue(vs);
            rapidjson::Value argsCopy; argsCopy.CopyFrom(*opInfo.args, alloc);
            rapidjson::Value srcCopy;  srcCopy.CopyFrom(*source,       alloc);
            return std::make_shared<rapidjson::Value>(
                buildXmlForOperator(ud.ctx, opInfo.name, argsCopy, srcCopy));
        };

        // cat: timeline-aligned (horizontal-cycle) pattern
        parser_["cat"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            auto  arr = collectArray(vs, ud.ctx.document().GetAllocator());
            return std::make_shared<rapidjson::Value>(buildXmlForPattern(ud.ctx, arr, "t"));
        };
        // stack_op: vertical pattern
        parser_["stack_op"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud  = std::any_cast<UserData&>(dt);
            auto  arr = collectArray(vs, ud.ctx.document().GetAllocator());
            return std::make_shared<rapidjson::Value>(buildXmlForPattern(ud.ctx, arr, "v"));
        };
        parser_["group_operator"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };
        parser_["grouped_expr"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };

        // ── S3.6: command actions ─────────────────────────────────────────────

        parser_["hush"] = [](const peg::SemanticValues&, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            return std::make_shared<rapidjson::Value>(buildXmlForCommand(ud.ctx, "hush", {}));
        };
        parser_["setcps"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            float val = 0.f;
            for (const auto& v : vs)
                if (v.type() == typeid(double)) { val = float(std::any_cast<double>(v)); break; }
            return std::make_shared<rapidjson::Value>(buildXmlForCommand(ud.ctx, "setcps", val));
        };
        parser_["setbpm"] = [](const peg::SemanticValues& vs, std::any& dt) -> std::any {
            auto& ud = std::any_cast<UserData&>(dt);
            float val = 0.f;
            for (const auto& v : vs)
                if (v.type() == typeid(double)) { val = float(std::any_cast<double>(v)); break; }
            return std::make_shared<rapidjson::Value>(
                buildXmlForCommand(ud.ctx, "setcps", val / 120.f / 2.f));
        };
        parser_["command"] = [](const peg::SemanticValues& vs, std::any&) -> std::any {
            return firstPValue(vs);
        };
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
