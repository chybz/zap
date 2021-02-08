#include <zap/layouts/app.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/file_utils.hpp>

namespace zap::layouts {

static zap::string_map app_sub_dirs = {
    { "src", "src" }, // base directory for all sub directories below
    { "inc", "include" },
    { "bin", "bin" },
    { "lib", "lib" },
    { "mod", "mod" },
    { "tst", "tests" }
};

app::app(const std::string& project_dir)
: app("application", project_dir, app_sub_dirs)
{}

app::app(
    const std::string& label,
    const std::string& project_dir,
    const zap::string_map& sub_dirs
)
: zap::layout(label, project_dir),
sub_dirs_(sub_dirs)
{
    zap::die_if(
        sub_dirs_.count("src") == 0,
        "base source directory is not set"
    );

    dirs_ = {
        zap::target_src_dir(sub_dirs, zap::target_type::bin),
        zap::target_src_dir(sub_dirs, zap::target_type::lib),
        zap::target_src_dir(sub_dirs, zap::target_type::mod),
        zap::target_src_dir(sub_dirs, zap::target_type::tst)
    };
}

app::~app()
{}

bool
app::detect() const
{
    return
        detect_libs()
        ||
        detect_self_contained(zap::target_type::bin)
        ||
        detect_self_contained(zap::target_type::mod)
        ;
}

bool
app::detect_libs() const
{
    auto inc_dir = zap::cat_dir(
        project_dir_,
        sub_dirs_.at("src"),
        sub_dirs_.at("inc")
    );

    for (const auto& d : zap::find_dirs(inc_dir)) {
        bool has = zap::has_files(
            zap::cat_dir(inc_dir, d),
            zap::re(zap::re_type::hdr)
        );

        if (has) {
            return true;
        }
    }

    return false;
}

bool
app::detect_self_contained(zap::target_type type) const
{
    const auto& src_dir = dirs_.at(type);

    for (const auto& d : zap::find_dirs(src_dir)) {
        bool has = zap::has_files(
            zap::cat_dir(src_dir, d),
            zap::re(zap::re_type::src_or_hdr)
        );

        if (has) {
            return true;
        }
    }

    return false;
}

void
app::find_targets(zap::project& p)
{
    p.sub_targets = true;

    // We want paths to be relative to project root
    zap::call_in_directory(
        project_dir_,
        [&] {
            find_libs(p);
            find_targets(p, zap::target_type::mod, p.mods);
            find_targets(p, zap::target_type::bin, p.bins);
            find_targets(p, zap::target_type::tst, p.tsts);
        }
    );
}

void
app::find_libs(zap::project& p)
{
    auto inc_dir = zap::cat_dir(sub_dirs_.at("src"), sub_dirs_.at("inc"));
    const auto& src_dir = dirs_.at(zap::target_type::lib);

    zap::string_set candidates;

    auto dirs = zap::find_dirs(inc_dir);
    candidates.insert(dirs.begin(), dirs.end());
    dirs = zap::find_dirs(src_dir);
    candidates.insert(dirs.begin(), dirs.end());

    const auto& hdr = zap::re(zap::re_type::hdr);
    const auto& src = zap::re(zap::re_type::src);

    for (auto& lib : candidates) {
        zap::target t{
            lib,
            zap::target_type::lib,
            zap::cat_dir(src_dir, lib),
            zap::cat_dir(inc_dir, lib)
        };

        zap::add_files(t.public_headers, t.inc_dir, hdr);
        zap::add_files(t.private_headers, t.src_dir, hdr);
        zap::add_files(t.sources, t.src_dir, src);

        if (t.public_headers.empty()) {
            // Libraries must have an interface
            continue;
        }

        p.inc_dirs.push_back(zap::cat_dir(project_dir_, t.inc_dir));
        p.add_target(p.libs, lib, t);
    }
}

void
app::find_targets(
    zap::project& p,
    zap::target_type type,
    zap::targets& targets
)
{
    const auto& src_dir = dirs_.at(type);
    const auto& hdr = zap::re(zap::re_type::hdr);
    const auto& src = zap::re(zap::re_type::src);

    for (auto& d : zap::find_dirs(src_dir)) {
        zap::target t{ d, type, zap::cat_dir(src_dir, d) };

        zap::add_files(t.private_headers, t.src_dir, hdr);
        zap::add_files(t.sources, t.src_dir, src);

        if (t.sources.empty()) {
            // Must have something to compile
            continue;
        }

        p.add_target(targets, d, t);
    }
}

}
