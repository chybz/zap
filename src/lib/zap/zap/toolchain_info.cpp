#include <unordered_map>

#include <re2/re2.h>

#include <zap/toolchain_info.hpp>
#include <zap/utils.hpp>
#include <zap/types.hpp>

namespace zap {

bool
toolchain_info::link_shared() const
{ return link == link_type::shared_; }

bool
toolchain_info::link_static() const
{ return link == link_type::static_; }

struct toolchain_detect
{
    toolchain_type type;
    strings version_args;
    std::string re;
};

bool
identify(
    const toolchain_detect& d,
    const std::string& text,
    toolchain_info& ti
)
{
    auto vlines = split_lines(text);
    re2::RE2 re(d.re);
    re2::StringPiece match;

    for (const auto& vl : vlines) {
        if (re2::RE2::FullMatch(vl, re, &match)) {
            ti.type = d.type;
            ti.version.assign(match.begin(), match.end());

            return true;
        }
    }

    return false;
}

bool
identify(
    const toolchain_detect& d,
    const prog_result& r,
    toolchain_info& ti
)
{ return identify(d, r.out, ti) || identify(d, r.err, ti); }

void
detect_toolchain(toolchain_info& ti)
{
    static std::vector<toolchain_detect> detectors = {
        {
            toolchain_type::gcc,
            { "-v" },
            "gcc version (\\S+).*"
        },
        {
            toolchain_type::clang,
            { "-v" },
            "clang version (\\S+).*"
        },
        {
            toolchain_type::apple_clang,
            { "-v" },
            "Apple LLVM version (\\S+).*"
        },
        {
            toolchain_type::msvc,
            { "/?" },
            "Microsoft .*C/C++ Optimizing Compiler Version (\\S+) .*"
        }
    };

    for (const auto& d : detectors) {
        auto r = ti.cxx.run({
            .args = d.version_args,
            .opts = {
                .mode = run_mode::no_fail,
                .redirect = false
            }
        });

        if (identify(d, r, ti)) {
            break;
        }
    }
}

}
