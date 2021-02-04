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
#include <zap/resolvers/apt.hpp>

namespace zap::command {

void
scan::operator()(const toolchain& tc)
{
    tc_ptr_ = std::addressof(tc);

    //zap::make_resolvers(tc, rps_);

    find_targets();
    scan_targets();
    resolve_targets();

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
scan::resolve_targets()
{
    zap::resolvers::apt apt(tc());
    zap::resolve_info ri;

    resolve_targets(apt, p_.libs, ri);
    resolve_targets(apt, p_.mods, ri);
    resolve_targets(apt, p_.bins, ri);
    resolve_targets(apt, p_.tsts, ri);

    p_.finalize();

    project_info(std::cout, apt, ri);
}

void
scan::scan_targets(zap::targets& ts)
{
    auto cb = [&](auto index, auto& target) {
        scan_target(target);
    };

    zap::async_pool<decltype(cb), cmake_config_context> ap(tc().exec(), cb);

    for (auto& p : ts) {
        scan_target(p.second);
    }
}

void
scan::scan_target(zap::target& t)
{
    zap::log("scanning ", t.type, " ", t.name);
    scan_target_files(t, t.inc_dir, t.public_headers, t.public_deps);
    scan_target_files(t, t.src_dir, t.private_headers, t.private_deps);
    scan_target_files(t, t.src_dir, t.sources, t.private_deps);
}

void
scan::scan_target_files(
    zap::target& t,
    const std::string& dir,
    const zap::files& files,
    zap::target_deps& deps
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
            deps.project_libs.insert(lib);
        } else {
            deps.headers.insert(dep);
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

void
scan::resolve_targets(
    const zap::resolver& res,
    zap::targets& ts,
    zap::resolve_info& ri
)
{
    for (auto& p : ts) {
        resolve_deps(res, p.second, ri);
    }
}

void
scan::resolve_deps(
    const zap::resolver& res,
    zap::target& t,
    zap::resolve_info& ri
)
{
    resolve_header_deps(res, t.public_deps, t.pkg_deps, ri);
    resolve_header_deps(res, t.private_deps, t.pkg_deps, ri);

    t.normalize_libs();
}

void
scan::resolve_header_deps(
    const zap::resolver& res,
    zap::target_deps& deps,
    zap::string_set& pkgs,
    zap::resolve_info& ri
)
{
    for (const auto& dep : deps.headers) {
        auto di = res.resolve(dep);

        if (di.not_found()) {
            ri.unresolved_headers.insert(dep);
        } else if (di.found()) {
            if (di.has_pkgs()) {
                pkgs.insert(di.pkgs.begin(), di.pkgs.end());
                ri.used.insert(di.pkgs.begin(), di.pkgs.end());
                std::size_t installed_count = 0;

                for (const auto& p : di.pkgs) {
                    if (!res.installed(p)) {
                        ri.to_install.insert(p);
                    } else {
                        ++installed_count;
                    }
                }

                if (installed_count == di.pkgs.size()) {
                    add_project_module(di);

                    deps.add_libs(di.module.targets);
                    deps.add_libs(di.raw_libs);
                }
            } else if (di.has_pkg_candidates()) {
                ri.to_choose.merge(di.pkg_candidates);
            }
        } else if (di.ambiguous()) {
            ri.ambiguous_headers.try_emplace(
                dep, std::move(di.pkg_candidates)
            );
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
    } else if (info.is_raw()) {
        p_.raw_libs.insert(info.raw_libs.begin(), info.raw_libs.end());
    }
}

void
scan::project_info(
    std::ostream& os,
    const zap::resolver& res,
    const zap::resolve_info& ri
) const
{
    if (ri.empty()) {
        return;
    }

    os << "Package info from " << res.name() << ":\n\n";

    project_pkg_info(os, "used", ri.used);
    project_pkg_info(os, "to install", ri.to_install);
    project_pkg_info(os, "to choose", ri.to_choose);
    project_pkg_info(os, "unresolved headers", ri.unresolved_headers);

    project_targets_info(os, "", p_.libs);
    project_targets_info(os, "", p_.bins);
    project_targets_info(os, "", p_.mods);
    project_targets_info(os, "", p_.tsts);
}

void
scan::project_pkg_info(
    std::ostream& os,
    const std::string& label,
    const zap::string_set& pkgs
) const
{
    if (pkgs.empty()) {
        return;
    }

    os << label << ": ";

    zap::string_views names;

    for (const auto& p : pkgs) {
        names.emplace_back(p);
    }

    os << zap::join(", ", names) << "\n";
}

void
scan::project_targets_info(
    std::ostream& os,
    const std::string& label,
    const zap::targets& ts
) const
{
    for (const auto& p : ts) {
        os << p.second << "\n\n";
    }
}

const zap::toolchain&
scan::tc() const
{ return *tc_ptr_; }

}
