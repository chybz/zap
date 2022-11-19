#include <memory>
#include <unordered_map>

#include <zap/resolver.hpp>
#include <zap/resolvers/apt.hpp>
#include <zap/utils.hpp>

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// Resolver context
//
///////////////////////////////////////////////////////////////////////////////
void
resolver_data::merge(resolver_data& other)
{
    zap::merge_pkg_items(headers, other.headers);
    zap::merge_pkg_items(pkg_config_names, other.pkg_config_names);
    pkg_config_to_pkg.merge(other.pkg_config_to_pkg);
    zap::merge_pkg_items(cmake_config_names, other.cmake_config_names);
    cmake_config_to_pkg.merge(other.cmake_config_to_pkg);
    zap::merge_pkg_items(lib_names, other.lib_names);
}

void
resolver_data::clear_temporaries()
{
    headers.clear();
    pkg_config_names.clear();
    pkg_config_to_pkg.clear();
    cmake_config_names.clear();
    cmake_config_to_pkg.clear();
    lib_names.clear();
}

///////////////////////////////////////////////////////////////////////////////
//
// Resolver
//
///////////////////////////////////////////////////////////////////////////////
resolver::resolver(
    const zap::env& e,
    const std::string& name,
    const std::string& root
)
: e_(e),
name_(name),
root_(root),
pc_(e_, root_),
cmc_(e_, root_)
{
    for (auto& d : e_.toolchain().make_arch_dirs(root_, "include")) {
        d.push_back('/');
        std_inc_dirs_.insert(std::move(d));
    }
}

resolver::~resolver()
{}

const zap::env&
resolver::env() const
{ return e_; }

const std::string&
resolver::name() const
{ return name_; }

const std::string&
resolver::root() const
{ return root_; }

const zap::pkg_config::configs&
resolver::pkg_configs() const
{ return pc_; }

zap::pkg_config::configs&
resolver::pkg_configs()
{ return pc_; }

const zap::cmake::configs&
resolver::cmake_configs() const
{ return cmc_; }

zap::cmake::configs&
resolver::cmake_configs()
{ return cmc_; }

zap::dep_info
resolver::resolve(const std::string& header) const
{
    auto hit = data_.header_to_dep.find(header);

    if (hit != data_.header_to_dep.end()) {
        return hit->second;
    }

    zap::string_set candidates;
    zap::dep_info ldi;
    auto th = "/" + header;

    // TODO: if too slow use some kind of suffix tree
    for (const auto& di : data_.file_headers) {
        if (di.file.ends_with(th)) {
            if (ldi.file.empty()) {
                // First match
                ldi = di;
            } else {
                candidates.insert(di.pkgs.begin(), di.pkgs.begin());
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
resolver::installed(const std::string& pkg) const
{ return installed_.contains(pkg); }

zap::string_set&
resolver::installed_set()
{ return installed_; }

const zap::string_set&
resolver::installed_set() const
{ return installed_; }

void
resolver::make_deps()
{
    for (const auto& hp : data_.headers) {
        const auto& pkg = hp.first;

        if (data_.cmake_config_names.contains(pkg)) {
            process_pkg_headers(
                pkg,
                data_.cmake_config_names,
                data_.cmake_config_to_pkg,
                cmc_
            );
        } else if (data_.pkg_config_names.contains(pkg)) {
            process_pkg_headers(
                pkg,
                data_.pkg_config_names,
                data_.pkg_config_to_pkg,
                pc_
            );
        } else {
            strip_pkg_headers(pkg, std_inc_dirs_);
        }
    }

    data_.clear_temporaries();

    zap::normalize_deps(data_.header_to_dep, installed_);
}

zap::resolver_data&
resolver::data()
{ return data_; }

void
resolver::process_pkg_headers(
    const std::string& pkg,
    const zap::pkg_items_map& config_names,
    const string_set_map& config_to_pkg,
    const zap::package_configs& configs
)
{
    const auto& names = config_names.at(pkg);

    for (const auto& config_name : names) {
        if (!configs.has(config_name)) {
            // config file is not installed
            continue;
        }

        strip_pkg_headers(
            pkg,
            configs,
            config_to_pkg,
            config_name
        );
    }
}

void
resolver::set_unresolved(
    const std::string& pkg,
    std::string& header
)
{
    zap::dep_info di{
        zap::dep_status::found,
        { pkg },
        {},
        package_config_type::unknown,
        std::move(header)
    };

    data_.file_headers.emplace_back(std::move(di));
}

void
resolver::strip_pkg_headers(
    const std::string& pkg,
    const package_configs& configs,
    const string_set_map& config_to_pkg,
    const std::string& config_name
)
{
    for (auto& h : data_.headers.at(pkg)) {
        if (h.empty()) {
            // Header already moved by other config of same package
            continue;
        }

        configs.strip_header(std_inc_dirs_, config_name, h);

        // Header was made relative to include directive
        auto& di = data_.header_to_dep[h];

        configs.header_to_module(config_name, h, di.module);
        di.status = zap::dep_status::found;
        di.config_type = configs.type();
        di.pkg_candidates.insert(pkg);

        if (
            di.module.config != config_name
            &&
            config_to_pkg.contains(di.module.config)
        ) {
            const auto& pkgs = config_to_pkg.at(di.module.config);

            di.pkg_candidates.insert(pkgs.begin(), pkgs.end());
        }
    }
}

void
resolver::strip_pkg_headers(
    const std::string& pkg,
    const zap::inc_dir_set& inc_dirs
)
{
    for (auto& h : data_.headers.at(pkg)) {
        if (strip_header(inc_dirs, h)) {
            auto &di = data_.header_to_dep[std::move(h)];

            di.status = zap::dep_status::found;
            di.config_type = package_config_type::raw;
            di.pkg_candidates.insert(pkg);

            if (data_.lib_names.contains(pkg)) {
                const auto& libs = data_.lib_names[pkg];

                di.raw_libs.insert(libs.begin(), libs.end());
            }
        } else {
            set_unresolved(pkg, h);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
void
make_resolvers(const zap::env& e, resolver_ptrs& rps)
{
    if (e.os_info().is_debian()) {
        auto rp = new_resolver<zap::resolvers::apt>(e);
        rps.emplace_back(std::move(rp));
    }
}

resolver_ptrs
make_resolvers(const zap::env& e)
{
    resolver_ptrs rps;

    make_resolvers(e, rps);

    return rps;
}

}
