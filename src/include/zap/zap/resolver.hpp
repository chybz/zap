#pragma once

#include <string>
#include <memory>
#include <vector>

#include <zap/toolchain.hpp>
#include <zap/dep_info.hpp>
#include <zap/pkg_configs.hpp>
#include <zap/cmake_configs.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/pkg_items.hpp>
#include <zap/types.hpp>

namespace zap {

struct resolver_data
{
    pkg_items_map headers;
    pkg_items_map pkg_config_names;
    pkg_items_map cmake_names;
    dep_info_map header_to_dep;
    dep_infos file_headers;

    void merge(resolver_data& other);
};

class resolver
{
public:
    resolver(
        const toolchain& tc,
        const std::string& name,
        const std::string& root
    );

    virtual ~resolver();

    const toolchain& tc() const;

    const std::string& name() const;
    const std::string& root() const;

    const zap::pkg_configs& pkg_configs() const;
    const zap::cmake_configs& cmake_configs() const;

    dep_info resolve(const std::string& header) const;

    bool installed(const std::string& pkg) const;

    zap::string_set& installed_set();
    const zap::string_set& installed_set() const;

protected:
    void make_deps();

    resolver_data& data();

private:
    void process_pkg_headers(
        const std::string& pkg,
        const pkg_items_map& config_names,
        const package_configs& configs
    );

    void strip_pkg_headers(
        const inc_dir_set& inc_dirs,
        const std::string& pkg,
        const std::string& config_name = {},
        package_config_type config_type = package_config_type::unknown
    );

    const toolchain& tc_;
    std::string name_;
    std::string root_;
    zap::pkg_configs pc_;
    zap::cmake_configs cmc_;
    inc_dir_set std_inc_dirs_;
    zap::string_set installed_;
    resolver_data data_;
};

using resolver_ptr = std::unique_ptr<resolver>;
using resolver_ptrs = std::vector<resolver_ptr>;

void
make_resolvers(const toolchain& tc, resolver_ptrs& rps);

resolver_ptrs
make_resolvers(const toolchain& tc);

template <typename Resolver, typename... Args>
resolver_ptr
new_resolver(Args&&... args)
{ return std::make_unique<Resolver>(std::forward<Args>(args)...); }

}
