#pragma once

#include <set>
#include <unordered_map>

#include <zap/toolchain.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/package_configs.hpp>

namespace zap {

class pkg_configs : public package_configs
{
public:
    pkg_configs(const toolchain& tc, const std::string& root);
    virtual ~pkg_configs();

    bool has(const std::string& pc_name) const final;
    bool has_include_dirs(const std::string& pc_name) const final;
    const inc_dir_set& include_dirs(const std::string& pc_name) const final;

    void header_to_module(
        const std::string& name,
        const std::string& header,
        module_dep_info& module
    ) const final;

private:
    void load_configs();

    bool clean_dir(std::string_view& dir) const;

    prog pc_;
    zap::string_set names_;
    inc_dir_sets inc_dirs_;
};

}
