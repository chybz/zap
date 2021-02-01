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
dep_info::has_pkg() const
{ return !pkg.empty(); }

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
        auto& pkgs = di.pkg_candidates;

        if (pkgs.size() == 1) {
            // Normalize deps with only one package candidate
            di.pkg = std::move(pkgs.extract(pkgs.begin()).value());
        } else if (pkgs.size() >= 1) {
            // Make installed package the default
            std::size_t count = 0;
            const std::string* last_installed = nullptr;

            for (const auto& pkg : pkgs) {
                if (installed.contains(pkg)) {
                    ++count;
                    last_installed = std::addressof(pkg);
                }
            }

            if (count == 1) {
                di.pkg = *last_installed;
            }
        }
    }
}

}
