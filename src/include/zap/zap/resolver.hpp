#pragma once

#include <string>
#include <memory>
#include <vector>

#include <zap/env.hpp>
#include <zap/dep_info.hpp>
#include <zap/pkg_config/configs.hpp>
#include <zap/cmake/configs.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/pkg_items.hpp>
#include <zap/types.hpp>

namespace zap {

struct resolver_data
{
    pkg_items_map headers;
    pkg_items_map pkg_config_names;
    string_set_map pkg_config_to_pkg;
    pkg_items_map cmake_config_names;
    string_set_map cmake_config_to_pkg;
    pkg_items_map lib_names;
    dep_info_map header_to_dep;
    dep_infos file_headers;

    void merge(resolver_data& other);
    void clear_temporaries();
};

class resolver
{
public:
    resolver(
        const zap::env& e,
        const std::string& name,
        const std::string& root
    );

    virtual ~resolver();

    const zap::env& env() const;

    const std::string& name() const;
    const std::string& root() const;

    const zap::pkg_config::configs& pkg_configs() const;
    const zap::cmake::configs& cmake_configs() const;

    dep_info resolve(const std::string& header) const;

    bool installed(const std::string& pkg) const;

    zap::string_set& installed_set();
    const zap::string_set& installed_set() const;

protected:
    void make_deps();

    resolver_data& data();

    zap::pkg_config::configs& pkg_configs();
    zap::cmake::configs& cmake_configs();

private:
    void process_pkg_headers(
        const std::string& pkg,
        const pkg_items_map& config_names,
        const string_set_map& config_to_pkg,
        const package_configs& configs
    );

    void set_unresolved(const std::string& pkg, std::string& header);

    void strip_pkg_headers(
        const std::string& pkg,
        const package_configs& configs,
        const string_set_map& config_to_pkg,
        const std::string& config_name
    );

    void strip_pkg_headers(
        const std::string& pkg,
        const inc_dir_set& inc_dirs
    );

    const zap::env& e_;
    std::string name_;
    std::string root_;
    zap::pkg_config::configs pc_;
    zap::cmake::configs cmc_;
    inc_dir_set std_inc_dirs_;
    zap::string_set installed_;
    resolver_data data_;
};

using resolver_ptr = std::unique_ptr<resolver>;
using resolver_ptrs = std::vector<resolver_ptr>;

void
make_resolvers(const zap::env& e, resolver_ptrs& rps);

resolver_ptrs
make_resolvers(const zap::env& e);

template <typename Resolver, typename... Args>
resolver_ptr
new_resolver(Args&&... args)
{ return std::make_unique<Resolver>(std::forward<Args>(args)...); }

}
