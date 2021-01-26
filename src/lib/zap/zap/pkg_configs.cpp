#include <re2/re2.h>

#include <zap/pkg_configs.hpp>
#include <zap/utils.hpp>
#include <zap/executor.hpp>
#include <zap/inc_dirs.hpp>

namespace zap {

namespace detail {

struct pkg_config_context
{
    zap::string_set names;
    zap::inc_dir_sets inc_dirs;

    void merge(pkg_config_context& other)
    {
        names.merge(other.names);
        inc_dirs.merge(other.inc_dirs);
    }
};

}

pkg_configs::pkg_configs(const zap::toolchain& tc, const std::string& root)
: package_configs(tc, root, package_config_type::pkg_config),
pc_{ zap::find_cmd("pkg-config") }
{
    load_configs();
}

pkg_configs::~pkg_configs()
{}

bool
pkg_configs::has(const std::string& pc_name) const
{ return names_.count(pc_name) != 0; }

bool
pkg_configs::has_include_dirs(const std::string& pc_name) const
{
    bool ret = false;
    auto it = inc_dirs_.find(pc_name);

    if (it != inc_dirs_.end()) {
        ret = !it->second.empty();
    }

    return ret;
}

const inc_dir_set&
pkg_configs::include_dirs(const std::string& pc_name) const
{ return inc_dirs_.at(pc_name); }

void
pkg_configs::header_to_module(
    const std::string& name,
    const std::string& header,
    module_dep_info& module
) const
{}

void
pkg_configs::load_configs()
{
    // I promise I tried *very* hard to have this right, without too much
    // kludge.
    // There are just too many broken .pc files not respecting variables and
    // other niceties
    pc_.push_args({
        "--cflags-only-I",
        "--silence-errors"
    });

    re2::RE2 inc_dir_re("-I([\\w/.-]+)");

    auto cb = [&](auto& ctx, auto& pc) {
        auto pc_name = basename(pc, ".pc");

        ctx.names.insert(pc_name);

        auto pc_line = pc_.get_line({ pc_name });

        inc_dir_set idirs;
        re2::StringPiece input(pc_line);
        re2::StringPiece match;

        while (re2::RE2::FindAndConsume(&input, inc_dir_re, &match)) {
            std::string_view inc_dir{ match.begin(), match.end() };

            if (!clean_dir(inc_dir)) {
                continue;
            }

            std::string cleaned(inc_dir.size() + 1, 0);
            cleaned.assign(inc_dir.begin(), inc_dir.end());
            cleaned.push_back('/');

            idirs.insert(std::move(cleaned));
        }

        ctx.inc_dirs.try_emplace(std::move(pc_name), std::move(idirs));
    };

    using pool = zap::async_pool<decltype(cb), detail::pkg_config_context>;

    pool ap(tc().exec(), cb);

    for (const auto& dir : tc().make_arch_dirs(root(), "lib", "pkgconfig")) {
        for (auto& pc : find_files(dir, ".*\\.pc")) {
            ap.async(std::move(pc));
        }
    }

    auto& merged = ap.wait();

    names_ = std::move(merged.names);
    inc_dirs_ = std::move(merged.inc_dirs);
}

bool
pkg_configs::clean_dir(std::string_view& dir) const
{
    if (dir.ends_with('/')) {
        dir.remove_suffix(1);
    }

    return !dir.empty();
}

}
