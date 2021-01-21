#include <memory>
#include <unordered_map>

#include <zap/resolver.hpp>
#include <zap/resolvers/apt.hpp>
#include <zap/utils.hpp>

namespace zap {

void
resolver_data::merge(resolver_data& other)
{
    zap::merge_items(headers, other.headers);
    zap::merge_items(pkg_config_names, other.pkg_config_names);
    zap::merge_items(cmake_names, other.cmake_names);
}

resolver::resolver(
    const toolchain& tc,
    const std::string& name,
    const std::string& root
)
: tc_(tc),
name_(name),
root_(root),
pc_(tc_, root_),
cmc_(tc_, root_)
{
    for (auto& d : tc_.make_arch_dirs(root_, "include", "/")) {
        std_inc_dirs_.insert(std::move(d));
    }
}

resolver::~resolver()
{}

const toolchain&
resolver::tc() const
{ return tc_; }

const std::string&
resolver::name() const
{ return name_; }

const std::string&
resolver::root() const
{ return root_; }

const zap::pkg_configs&
resolver::pkg_configs() const
{ return pc_; }

const zap::cmake_configs&
resolver::cmake_configs() const
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

        if (data_.cmake_names.contains(pkg)) {
            process_pkg_headers(pkg, data_.cmake_names, cmc_);
        } else if (data_.pkg_config_names.contains(pkg)) {
            process_pkg_headers(pkg, data_.pkg_config_names, pc_);
        } else {
            strip_pkg_headers(std_inc_dirs_, pkg);
        }
    }

    data_.headers.clear();
    data_.pkg_config_names.clear();
    data_.cmake_names.clear();

    zap::normalize_deps(data_.header_to_dep, installed_);
}

zap::resolver_data&
resolver::data()
{ return data_; }

void
resolver::process_pkg_headers(
    const std::string& pkg,
    const zap::pkg_items_map& config_names,
    const zap::package_configs& configs
)
{
    const auto& names = config_names.at(pkg);

    for (const auto& config_name : names) {
        if (!configs.has(config_name)) {
            // config file is not installed
            continue;
        }

        if (configs.has_include_dirs(config_name)) {
            // config file declares some include directories
            strip_pkg_headers(
                configs.include_dirs(config_name),
                pkg,
                config_name,
                configs.type()
            );
        } else {
            strip_pkg_headers(std_inc_dirs_, pkg, config_name);
        }
    }
}

void
resolver::strip_pkg_headers(
    const zap::inc_dir_set& inc_dirs,
    const std::string& pkg,
    const std::string& config_name,
    zap::package_config_type config_type
)
{
    for (auto& h : data_.headers.at(pkg)) {
        bool relative = false;

        if (h.ends_with("karma_uint.hpp")) {
            std::cout << "YALLAH" << std::endl;
        }

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
            auto &di = data_.header_to_dep[std::move(h)];

            di.status = zap::dep_status::found;

            if (!config_name.empty()) {
                di.config_name = config_name;
                di.config_type = config_type;
            }

            di.pkg_candidates.insert(pkg);
        } else {
            zap::dep_info di{
                zap::dep_status::found,
                pkg,
                {},
                package_config_type::unknown,
                std::move(h)
            };

            data_.file_headers.emplace_back(std::move(di));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
void
make_resolvers(const toolchain& tc, resolver_ptrs& rps)
{
    if (tc.os_info().is_debian()) {
        auto rp = new_resolver<zap::resolvers::apt>(tc);
        rps.emplace_back(std::move(rp));
    }
}

resolver_ptrs
make_resolvers(const toolchain& tc)
{
    resolver_ptrs rps;

    make_resolvers(tc, rps);

    return rps;
}

}
