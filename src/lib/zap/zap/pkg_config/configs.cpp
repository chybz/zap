#include <re2/re2.h>

#include <zap/pkg_config/configs.hpp>
#include <zap/utils.hpp>
#include <zap/executor.hpp>
#include <zap/inc_dirs.hpp>

namespace zap::pkg_config {

namespace detail {

struct config_context
{
    zap::string_set names;
    zap::inc_dir_sets inc_dirs;

    void merge(config_context& other)
    {
        names.merge(other.names);
        inc_dirs.merge(other.inc_dirs);
    }
};

}

configs::configs(const zap::env& e, const std::string& root)
: package_configs(e, root, zap::package_config_type::pkg_config),
pc_{ zap::find_cmd("pkg-config") }
{
    set_config_paths("pkgconfig");

    pc_.env["PKG_CONFIG_PATH"] = join(":", config_paths());

    load_configs();
}

configs::~configs()
{}

bool
configs::has(const std::string& pc_name) const
{ return names_.count(pc_name) != 0; }

bool
configs::has_include_dirs(const std::string& pc_name) const
{
    bool ret = false;
    auto it = inc_dirs_.find(pc_name);

    if (it != inc_dirs_.end()) {
        ret = !it->second.empty();
    }

    return ret;
}

const zap::inc_dir_set&
configs::include_dirs(const std::string& pc_name) const
{ return inc_dirs_.at(pc_name); }

void
configs::header_to_module(
    const std::string& name,
    const std::string& header,
    zap::module_dep_info& module
) const
{
    module.name = name;
    module.targets.push_back("PkgConfig::" + name);
}

void
configs::load_configs()
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

        auto pc_line = pc_.get_line({ .args = { pc_name } });

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

    using pool = zap::async_pool<decltype(cb), detail::config_context>;

    pool ap(env().executor(), cb);

    auto conf_dirs = make_config_paths(
        "pkgconfig",
        package_config_mode::private_
    );

    for (const auto& dir : conf_dirs) {
        for (auto& pc : find_files(dir, ".*\\.pc")) {
            ap.async(std::move(pc));
        }
    }

    auto& merged = ap.wait();

    names_ = std::move(merged.names);
    inc_dirs_ = std::move(merged.inc_dirs);
}

bool
configs::clean_dir(std::string_view& dir) const
{
    if (dir.ends_with('/')) {
        dir.remove_suffix(1);
    }

    return !dir.empty();
}

}
