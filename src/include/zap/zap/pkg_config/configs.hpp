#pragma once

#include <set>
#include <unordered_map>

#include <zap/env.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/package_configs.hpp>

namespace zap::pkg_config {

class configs : public zap::package_configs
{
public:
    configs(const zap::env& e, const std::string& root);
    virtual ~configs();

    bool has(const std::string& pc_name) const final;
    bool has_include_dirs(const std::string& pc_name) const final;
    const zap::inc_dir_set& include_dirs(const std::string& pc_name) const final;

    void header_to_module(
        const std::string& name,
        const std::string& header,
        zap::module_dep_info& module
    ) const final;

private:
    void load_configs();

    bool clean_dir(std::string_view& dir) const;

    prog pc_;
    zap::string_set names_;
    zap::inc_dir_sets inc_dirs_;
};

}
