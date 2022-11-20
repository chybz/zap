#include <string>
#include <iostream>
#include <unordered_set>
#include <filesystem>

#include <zap/commands/configure.hpp>
#include <zap/env.hpp>
#include <zap/layout.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/generators/cmake.hpp>

namespace zap::commands {

configure::configure(const zap::env& e, const configure_opts& opts)
: zap::command(e),
opts_(opts)
{}

configure::~configure()
{}

void
configure::operator()()
{
    find_targets();
    scan_targets();

    zap::generators::cmake cm(env(), p_);

    cm.generate();
}

void
configure::find_targets()
{
    p_.root_dir = std::filesystem::current_path();

    auto& layout = zap::get_layout(p_.root_dir);

    layout.find_targets(p_);
}

void
configure::scan_targets()
{
    scan_targets(p_.libs);
    scan_targets(p_.mods);
    scan_targets(p_.bins);
    scan_targets(p_.tsts);
}

void
configure::scan_targets(zap::targets& ts)
{
    /*auto cb = [&](auto index, auto& target) {
        scan_target(target);
    };

    zap::async_pool<
        decltype(cb),
        cmake::config_context
    > ap(env().executor(), cb);

    for (auto& p : ts) {
        scan_target(p.second);
    }*/
}

void
configure::scan_target(zap::target& t)
{
    zap::log("scanning ", t.type, " ", t.name);
    scan_target_files(t, t.inc_dir, t.public_headers, t.public_deps);
    scan_target_files(t, t.src_dir, t.private_headers, t.private_deps);
    scan_target_files(t, t.src_dir, t.sources, t.private_deps);
}

void
configure::scan_target_files(
    zap::target& t,
    const std::string& dir,
    const zap::files& files,
    zap::target_deps& deps
)
{
    zap::strings all_deps;
    const auto& tc = env().toolchain();

    tc.scan_files(p_.inc_dirs, dir, files, all_deps);

    std::string lib;

    for (auto& dep : all_deps) {
        if (tc.is_std_header(dep) || t.has_file(dep)) {
            continue;
        }

        if (is_project_dep(t, dep, lib)) {
            deps.project_libs.insert(lib);
        } else {
            deps.headers.insert(dep);
        }
    }
}

bool
configure::is_project_dep(
    const target& t,
    const std::string& dep,
    std::string& lib
) const
{
    lib.clear();

    for (const auto& p : p_.libs) {
        const auto& other = p.second;

        if (t == other) {
            // That's me
            continue;
        }

        if (other.has_public_header(dep)) {
            lib = other.name;
            return true;
        }
    }

    return false;
}

}
