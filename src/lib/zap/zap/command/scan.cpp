#include <string>
#include <iostream>
#include <unordered_set>
#include <filesystem>

#include <yaml-cpp/yaml.h>

#include <zap/command/scan.hpp>
#include <zap/toolchain.hpp>
#include <zap/layout.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::command {

void
scan::operator()(const toolchain& tc)
{
    tc_ptr_ = std::addressof(tc);
    //zap::make_resolvers(tc, rps_);
    find_targets();
    scan_targets();

    zap::resolvers::apt apt(tc);
    zap::resolve_info ri;

    for (auto& t : p_.libs) {
        resolve_deps(apt, t, ri);
    }

    std::cout << "scan done." << std::endl;
}

void
scan::find_targets()
{
    p_.root_dir = std::filesystem::current_path();

    auto& layout = zap::get_layout(p_.root_dir);

    layout.find_targets(p_);
}

void
scan::scan_targets()
{
    scan_targets(p_.libs);
    scan_targets(p_.mods);
    scan_targets(p_.bins);
    scan_targets(p_.tsts);
}

void
scan::scan_targets(zap::targets& ts)
{
    for (auto& t : ts) {
        scan_target(t);
    }
}

void
scan::scan_target(zap::target& t)
{
    zap::log("scanning ", t.type, " ", t.name);
    scan_target_files(t, t.inc_dir, t.public_headers, t.public_header_deps);
    scan_target_files(t, t.src_dir, t.private_headers, t.private_header_deps);
    scan_target_files(t, t.src_dir, t.sources, t.private_header_deps);
}

void
scan::scan_target_files(
    zap::target& t,
    const std::string& dir,
    const zap::files& files,
    zap::string_vector_map& deps
)
{
    zap::strings all_deps;

    tc().scan_files(p_.inc_dirs, dir, files, all_deps);

    std::string lib;

    for (auto& dep : all_deps) {
        if (tc().is_std_header(dep) || t.has_file(dep)) {
            continue;
        }

        if (is_project_dep(t, dep, lib)) {
            t.project_lib_deps.push_back(lib);
        } else {
            deps.push_back(dep);
        }
    }
}

bool
scan::is_project_dep(
    const target& t,
    const std::string& dep,
    std::string& lib
) const
{
    lib.clear();

    for (const auto& other : p_.libs) {
        if (t.name != other.name && other.has_public_header(dep)) {
            lib = other.name;
            return true;
        }
    }

    return false;
}

void
scan::resolve_deps(
    const zap::resolvers::apt& apt,
    zap::target& t,
    zap::resolve_info& ri
)
{
    resolve_header_deps(
        apt,
        t.public_header_deps,
        t.public_pkg_deps,
        t.public_lib_deps,
        ri
    );

    resolve_header_deps(
        apt,
        t.private_header_deps,
        t.private_pkg_deps,
        t.private_lib_deps,
        ri
    );
}

void
scan::resolve_header_deps(
    const zap::resolvers::apt& apt,
    const zap::string_vector_map& headers,
    zap::string_vector_map& pkgs,
    zap::string_set& libs,
    zap::resolve_info& ri
)
{
    for (const auto& dep : headers) {
        if (seen_deps_.contains(dep)) {
            continue;
        } else {
            seen_deps_.insert(dep);
        }

        auto di = apt.resolve(dep);

        if (di.not_found()) {
            ri.unresolved.insert(dep);
        } else if (di.found()) {
            if (di.has_pkg()) {
                pkgs.push_back(di.pkg);

                if (!apt.installed(di.pkg)) {
                    ri.to_install.insert(di.pkg);
                } else {
                    add_project_module(di);

                    libs.insert(
                        di.module.targets.begin(),
                        di.module.targets.end()
                    );
                }
            } else if (di.has_pkg_candidates()) {
                ri.to_choose.merge(di.pkg_candidates);
            }
        } else if (di.ambiguous()) {
            ri.ambiguous.try_emplace(dep, std::move(di.pkg_candidates));
        }
    }
}

void
scan::add_project_module(const zap::dep_info& info)
{
    if (info.is_cmake_component()) {
        p_.cmake_components[info.module.name].insert(info.module.component);
    } else if (info.is_cmake()) {
        p_.cmake_modules.insert(info.module.name);
    } else if (info.is_pkg_config()) {
        p_.pkg_configs.insert(info.module.name);
    }
}

const zap::toolchain&
scan::tc() const
{ return *tc_ptr_; }

}
