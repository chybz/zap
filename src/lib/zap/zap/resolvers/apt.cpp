#include <sstream>

#include <taskflow/taskflow.hpp>

#include <zap/resolvers/apt.hpp>
#include <zap/utils.hpp>
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
content_inc_pat(const zap::toolchain& tc)
{
    std::string inc_pat = "(usr/(?:local/)?include/";
    inc_pat += "(?:" + tc.target_arch() + "/)?";
    inc_pat += "\\S+)";
    inc_pat += "\\s+(.+)";

    return inc_pat;
}

std::string
content_pc_pat(const zap::toolchain& tc)
{
    std::string pc_pat = "usr/(?:local/)?lib/";
    pc_pat += "(?:" + tc.target_arch() + "/)?pkgconfig/";
    pc_pat += "(\\S+)\\.pc";
    pc_pat += "\\s+(.+)";

    return pc_pat;
}

std::string
content_cmake_pat(const zap::toolchain& tc)
{
    std::string cmake_pat = "usr/(?:local/)?lib/";
    cmake_pat += "(?:" + tc.target_arch() + "/)?cmake/";
    cmake_pat += "\\S+/(\\S+)(?:Config|-config)\\.cmake";
    cmake_pat += "\\s+(.+)";

    return cmake_pat;
}

std::string
content_pkg_pat(const zap::toolchain& tc)
{ return "/([^\\s/]+)(?:,|$)"; }

}

///////////////////////////////////////////////////////////////////////////////
//
// APT context
//
///////////////////////////////////////////////////////////////////////////////
void
apt_context::merge(apt_context& other)
{
    zap::merge_items(headers, other.headers);
    zap::merge_items(pkg_config_names, other.pkg_config_names);
    zap::merge_items(cmake_names, other.cmake_names);
}

///////////////////////////////////////////////////////////////////////////////
//
// APT resolver
//
///////////////////////////////////////////////////////////////////////////////
apt::apt(const zap::toolchain& tc)
: resolver("apt"),
tc_{ tc },
pc_(tc_, "/usr"),
cmc_(tc_, "/usr"),
inc_re_(detail::content_inc_pat(tc_)),
pc_re_(detail::content_pc_pat(tc_)),
cmake_re_(detail::content_cmake_pat(tc_)),
pkg_re_(detail::content_pkg_pat(tc_))
{
    std_inc_dirs_.insert("/usr/include/");
    std_inc_dirs_.insert("/usr/include/" + tc_.target_arch() + "/");

    load_installed();
    load_contents();
}

apt::~apt()
{}

zap::dep_info
apt::resolve(const std::string& header) const
{
    const auto& c = ctxs_.front();

    auto hit = c.header_to_dep.find(header);

    if (hit != c.header_to_dep.end()) {
        return hit->second;
    }

    zap::string_set candidates;
    zap::dep_info ldi;
    auto th = "/" + header;

    // TODO: if too slow use some kind of suffix tree
    for (const auto& di : c.file_headers) {
        if (di.file.ends_with(th)) {
            if (ldi.file.empty()) {
                // First match
                ldi = di;
            } else {
                candidates.insert(di.pkg);
            }
        }
    }

    if (candidates.size() > 1) {
        ldi.status = zap::dep_status::ambiguous;
        ldi.pkg_candidates = std::move(candidates);
    }

    return ldi;
}

bool
apt::installed(const std::string& pkg) const
{ return installed_.contains(pkg); }

void
apt::load_contents()
{
    // TODO: backport to older dist where this was gzipped files
    std::string gpat{ "/var/lib/apt/lists/*Contents-*.lz4" };

    ctxs_.resize(tc_.exec().num_workers());

    for (auto&& f : zap::glob(gpat)) {
        parse_contents(f);
    }

    // TODO: replace this with executor auto contexts and fix parse_contents
    // Merge all maps into first one
    while (ctxs_.size() != 1) {
        ctxs_.front().merge(ctxs_.back());
        ctxs_.pop_back();
    }

    make_deps(ctxs_.front());
}

void
apt::parse_contents(const std::string& file)
{
    archive_util au(file);

    au.for_each_line_block(
        tc_.exec(),
        [&](std::size_t index, const auto& data) {
            std::string_view lines(data.begin(), data.end());

            re2::StringPiece inc_match;
            std::string item;
            std::string pkgs;

            auto& ctx = ctxs_[index];
            auto& hm = ctx.headers;
            auto& pm = ctx.pkg_config_names;
            auto& cm = ctx.cmake_names;

            for (const auto& l : zap::split_lines(lines)) {
                if (re2::RE2::FullMatch(l, inc_re_, &inc_match, &pkgs)) {
                    item.clear();
                    item.reserve(inc_match.size() + 1);
                    item.push_back('/');
                    item.append(inc_match.begin(), inc_match.end());

                    add_pkg_item(hm, item, pkgs);
                } else if (re2::RE2::FullMatch(l, pc_re_, &item, &pkgs)) {
                    add_pkg_item(pm, item, pkgs);
                } else if (re2::RE2::FullMatch(l, cmake_re_, &item, &pkgs)) {
                    add_pkg_item(cm, item, pkgs);
                }
            }
        }
    );
}

void
apt::add_pkg_item(
    zap::pkg_items_map& m,
    const std::string& item,
    const std::string& pkgs
)
{
    re2::StringPiece input(pkgs);
    re2::StringPiece match;

    while (re2::RE2::FindAndConsume(&input, pkg_re_, &match)) {
        std::string pkg(match.begin(), match.end());

        m[pkg].emplace_back(item);
    }
}

void
apt::make_deps(apt_context& ctx)
{
    for (const auto& hp : ctx.headers) {
        const auto& pkg = hp.first;

        auto it = ctx.pkg_config_names.find(pkg);

        if (it != ctx.pkg_config_names.end()) {
            const auto& pkg_config_names = it->second;

            for (const auto& pcname : pkg_config_names) {
                if (!pc_.has(pcname)) {
                    // pkg-config file is not installed
                    continue;
                }

                process_pkg_headers(ctx, pkg, pcname);
            }
        } else {
            process_pkg_headers(ctx, pkg);
        }
    }

    ctx.headers.clear();
    ctx.pkg_config_names.clear();

    zap::normalize_deps(ctx.header_to_dep, installed_);
}

void
apt::process_pkg_headers(
    apt_context& ctx,
    const std::string& pkg,
    const std::string& pcname
)
{
    if (!pcname.empty() && pc_.has_include_dirs(pcname)) {
        strip_pkg_headers(ctx, pc_.include_dirs(pcname), pkg, pcname);
    } else {
        strip_pkg_headers(ctx, std_inc_dirs_, pkg, pcname);
    }
}

void
apt::strip_pkg_headers(
    apt_context& ctx,
    const std::set<std::string>& inc_dirs,
    const std::string& pkg,
    const std::string& pcname
)
{
    for (auto& h : ctx.headers.at(pkg)) {
        bool relative = false;

        // Note: longest to shortest path
        for (const auto& inc_dir : zap::reverse(inc_dirs)) {
            if (h.starts_with(inc_dir)) {
                // Only one directory should match
                h.erase(0, inc_dir.size());
                relative = true;
                break;
            }
        }

        if (relative) {
            // Header was made relative to include directive
            auto &di = ctx.header_to_dep[std::move(h)];

            di.status = zap::dep_status::found;

            if (!pcname.empty()) {
                di.pc = pcname;
            }

            di.pkg_candidates.insert(pkg);
        } else {
            zap::dep_info di{
                zap::dep_status::found,
                pkg,
                {},
                std::move(h)
            };

            ctx.file_headers.emplace_back(std::move(di));
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

                installed_.insert(std::move(pkg));
            }
        }
    );
}

}
