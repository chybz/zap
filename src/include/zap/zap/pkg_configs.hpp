#pragma once

#include <set>
#include <unordered_map>

#include <zap/toolchain.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>

namespace zap {

class pkg_configs
{
public:
    using inc_dir_set = std::set<std::string>;
    using inc_dir_sets = std::unordered_map<std::string, inc_dir_set>;

    pkg_configs(const toolchain& tc, const std::string& root);
    virtual ~pkg_configs();

    const zap::string_set& pc_names() const;

    bool has(const std::string& pc_name) const;
    bool has_include_dirs(const std::string& pc_name) const;
    const inc_dir_set& include_dirs(const std::string& pc_name) const;

private:
    void load_configs();

    bool clean_dir(std::string_view& dir) const;

    const toolchain& tc_;
    std::string root_;
    prog pc_;
    zap::string_set names_;
    inc_dir_sets inc_dirs_;
};

}
