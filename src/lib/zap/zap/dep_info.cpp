#include <zap/dep_info.hpp>

namespace zap {

bool
dep_info::not_found() const
{ return status == dep_status::not_found; }

bool
dep_info::found() const
{ return status == dep_status::found; }

bool
dep_info::ambiguous() const
{ return status == dep_status::ambiguous; }

bool
dep_info::has_pkgs() const
{ return !pkgs.empty(); }

bool
dep_info::has_pkg_candidates() const
{ return !pkg_candidates.empty(); }

bool
dep_info::is_cmake() const
{ return config_type == package_config_type::cmake; }

bool
dep_info::is_cmake_component() const
{
    return
        is_cmake()
        &&
        !module.component.empty()
        ;
}

bool
dep_info::is_pkg_config() const
{ return config_type == package_config_type::pkg_config; }

bool
dep_info::is_raw() const
{ return config_type == package_config_type::raw; }

void
normalize_deps(dep_info_map& m, const string_set& installed)
{
    for (auto& p : m) {
        auto& di = p.second;
        auto& candidates = di.pkg_candidates;

        if (candidates.size() == 1) {
            // Normalize deps with only one package candidate
            di.pkgs.insert(std::move(candidates.extract(candidates.begin())));
        } else if (candidates.size() >= 1) {
            // Make installed package the default
            for (const auto& pkg : candidates) {
                if (installed.contains(pkg)) {
                    di.pkgs.insert(pkg);
                }
            }
        }
    }
}

}
