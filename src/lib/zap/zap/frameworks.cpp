#include <zap/frameworks.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// header to library detector
//
///////////////////////////////////////////////////////////////////////////////
void
header_to_lib::match(const std::string& header, lib_info& li) const
{
    re2::StringPiece input(header);
    re2::StringPiece match;

    if (re2::RE2::FullMatch(input, re, &match)) {
        li.matched = true;

        cb(std::string_view{ match.data(), match.size() }, li);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Module
//
///////////////////////////////////////////////////////////////////////////////
void
module::add(const std::string& pat, header_match_cb cb)
{
    auto dp = std::make_unique<header_to_lib>(pat, cb);

    detectors.emplace_back(std::move(dp));
}

lib_info
module::match(const std::string& header) const
{
    lib_info li;

    for (const auto& dp : detectors) {
        dp->match(header, li);

        if (li.matched) {
            break;
        }
    }

    return li;
}

///////////////////////////////////////////////////////////////////////////////
//
// Frameworks
//
///////////////////////////////////////////////////////////////////////////////
frameworks::frameworks()
{
    auto& b = add_module("Boost", "boost/");
    auto bcb = [](const auto& m, auto& li) {
        li.component.assign(m.data(), m.size());
        li.lib_name = "libboost_";
        li.lib_name.append(m.data(), m.size());
    };

    b.add("boost/(\\w+)\\.hpp", bcb);
    b.add("boost/(\\w+)/\\w+\\.hpp", bcb);
}

frameworks::~frameworks()
{}

module_match_info
frameworks::match(const std::string& header) const
{
    module_match_info mmi;

    for (const auto& p : modules_) {
        const auto& prefix = p.first;
        const auto& m = p.second;

        if (header.starts_with(prefix)) {
            mmi.info = m.match(header);

            if (mmi.info.matched) {
                mmi.matched = true;
                mmi.module = m.name;
            }
        }

        if (mmi.matched) {
            break;
        }
    }

    return mmi;
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
