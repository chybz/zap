#include <sstream>

#include <taskflow/taskflow.hpp>

#include <zap/resolvers/apt.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/archive_utils.hpp>

///////////////////////////////////////////////////////////////////////////////
//
// Notice:
//
// This is ongoing work to stabilize detectors and library/dependency names
// Once done, a refactor will be needed to clean up specifics
//
///////////////////////////////////////////////////////////////////////////////

namespace zap::resolvers {

namespace detail {

std::string
content_inc_pat(const zap::env& e)
{
    std::string inc_pat = "(usr/(?:local/)?include/";
    inc_pat += "(?:" + e.toolchain().target_arch() + "/)?";
    inc_pat += "\\S+)";
    inc_pat += "\\s+(.+)";

    return inc_pat;
}

std::string
content_pc_pat(const zap::env& e)
{
    std::string pc_pat = "usr/(?:local/)?lib/";
    pc_pat += "(?:" + e.toolchain().target_arch() + "/)?pkgconfig/";
    pc_pat += "(\\S+)\\.pc";
    pc_pat += "\\s+(.+)";

    return pc_pat;
}

std::string
content_cmake_pat(const zap::env& e)
{
    std::string cmake_pat = "usr/(?:local/)?lib/";
    cmake_pat += "(?:" + e.toolchain().target_arch() + "/)?cmake/";
    cmake_pat += "\\S+/(\\S+)(?:Config|-config)\\.cmake";
    cmake_pat += "\\s+(.+)";

    return cmake_pat;
}

std::string
content_pkg_pat(const zap::env& e)
{ return "/([^\\s/]+)(?:,|$)"; }

std::string
content_lib_pat(const zap::env& e)
{
    std::string lib_pat = "usr/(?:local/)?lib/";
    lib_pat += "(?:" + e.toolchain().target_arch() + "/)?";
    lib_pat += "lib(\\S+)\\.(?:so|a)";
    lib_pat += "\\s+(.+)";

    return lib_pat;
}

}

///////////////////////////////////////////////////////////////////////////////
//
// APT resolver
//
///////////////////////////////////////////////////////////////////////////////
apt::apt(const zap::env& e)
: resolver(e, "apt", "/usr"),
inc_re_(detail::content_inc_pat(e)),
pc_re_(detail::content_pc_pat(e)),
cmake_re_(detail::content_cmake_pat(e)),
pkg_re_(detail::content_pkg_pat(e)),
lib_re_(detail::content_lib_pat(e))
{
    load_installed();
    load_contents();
}

apt::~apt()
{}

void
apt::load_contents()
{
    // TODO: backport to older dist where this was gzipped files
    std::string gpat{ "/var/lib/apt/lists/*Contents-*.lz4" };

    for (auto&& f : zap::glob(gpat)) {
        parse_contents(f);
    }

    make_deps();
}

void
apt::parse_contents(const std::string& file)
{
    archive_util<zap::resolver_data> au(file, env().executor());

    au.for_each_line_block(
        [&](auto& ctx, const auto& data) {
            std::string_view lines(data.begin(), data.end());

            re2::StringPiece inc_match;
            std::string item;
            std::string pkgs;

            auto& hm = ctx.headers;
            auto& pm = ctx.pkg_config_names;
            auto& p2p = ctx.pkg_config_to_pkg;
            auto& cm = ctx.cmake_config_names;
            auto& c2p = ctx.cmake_config_to_pkg;
            auto& lm = ctx.lib_names;

            for (const auto& l : zap::split_lines(lines)) {
                if (re2::RE2::FullMatch(l, inc_re_, &inc_match, &pkgs)) {
                    item.clear();
                    item.reserve(inc_match.size() + 1);
                    item.push_back('/');
                    item.append(inc_match.begin(), inc_match.end());

                    add_pkg_item(hm, item, pkgs);
                } else if (re2::RE2::FullMatch(l, pc_re_, &item, &pkgs)) {
                    add_pkg_item(pm, item, pkgs, &p2p);
                } else if (re2::RE2::FullMatch(l, cmake_re_, &item, &pkgs)) {
                    add_pkg_item(cm, item, pkgs, &c2p);
                } else if (re2::RE2::FullMatch(l, lib_re_, &item, &pkgs)) {
                    add_pkg_item(lm, item, pkgs);
                }
            }
        },
        data()
    );
}

void
apt::add_pkg_item(
    zap::pkg_items_map& m,
    const std::string& item,
    const std::string& pkgs,
    string_set_map* rev
)
{
    re2::StringPiece input(pkgs);
    re2::StringPiece match;

    while (re2::RE2::FindAndConsume(&input, pkg_re_, &match)) {
        std::string pkg(match.begin(), match.end());

        m[pkg].emplace_back(item);

        if (rev) {
            (*rev)[item].insert(pkg);
        }
    }
}

void
apt::load_installed()
{
    zap::prog dpkg_query {
        zap::find_cmd("dpkg-query"),
        { "-W", "-f", "${Package}\\n" }
    };

    dpkg_query.read_lines(
        [&](auto& lines) {
            for (auto& line : lines) {
                std::string pkg(line);

                installed_set().insert(std::move(pkg));
            }
        }
    );
}

}
