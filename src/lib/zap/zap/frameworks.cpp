#include <zap/frameworks.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// header to library detector
//
///////////////////////////////////////////////////////////////////////////////
bool
header_to_module::match(
    const std::string& header,
    const string_set_map& config_targets,
    module_match_info& info
) const
{
    re2::StringPiece input(header);
    re2::StringPiece match;

    if (re2::RE2::FullMatch(input, re, &match)) {
        info.matched = true;

        cb(
            std::string_view{ match.data(), match.size() },
            config_targets,
            info
        );
    }

    return info.matched;
}

///////////////////////////////////////////////////////////////////////////////
//
// Module
//
///////////////////////////////////////////////////////////////////////////////
void
module::add(const std::string& pat, header_match_cb cb)
{
    auto dp = std::make_unique<header_to_module>(pat, cb);

    detectors.emplace_back(std::move(dp));
}

bool
module::match(
    const std::string& header,
    const string_set_map& config_targets,
    module_match_info& info
) const
{
    for (const auto& dp : detectors) {
        if (dp->match(header, config_targets, info)) {
            return true;
        }
    }

    return info.matched;
}

///////////////////////////////////////////////////////////////////////////////
//
// Frameworks
//
///////////////////////////////////////////////////////////////////////////////
frameworks::frameworks()
{
    auto& b = add_module("Boost", "boost/");
    auto bcb = [](const auto& m, const auto& config_targets, auto& info) {
        info.module = "Boost";
        info.config = "boost_";
        info.config.append(m.data(), m.size());

        if (!config_targets.contains(info.config)) {
            info.component = "headers";
            info.config = "boost_headers";
        } else {
            info.component.assign(m.data(), m.size());
        }
    };

    b.add("boost/(\\w+)\\.hpp", bcb);
    b.add("boost/(\\w+)/.+\\.hpp", bcb);
}

frameworks::~frameworks()
{}

module_match_info
frameworks::match(
    const std::string& header,
    const string_set_map& config_targets
) const
{
    module_match_info info;

    for (const auto& p : modules_) {
        const auto& prefix = p.first;
        const auto& m = p.second;

        if (
            header.starts_with(prefix)
            &&
            m.match(header, config_targets, info)
        ) {
            break;
        }
    }

    return info;
}

module&
frameworks::add_module(const std::string& name, const std::string& prefix)
{
    die_if(
        modules_.contains(name),
        "module ", name, " already exists"
    );

    auto p = modules_.try_emplace(prefix, module{ name });

    return p.first->second;
}

}
