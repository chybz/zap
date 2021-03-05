#pragma once

#include <set>
#include <unordered_map>

#include <zap/env.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/dep_info.hpp>
#include <zap/package_config_type.hpp>

namespace zap {

bool
strip_header(const inc_dir_set& inc_dirs, std::string& header);

enum class package_config_mode
{
    all_,
    public_,
    private_
};

class package_configs
{
public:
    package_configs(
        const zap::env& e,
        const std::string& root,
        package_config_type type
    );

    virtual ~package_configs();

    package_config_type type() const;

    void strip_header(
        const inc_dir_set& default_dirs,
        const std::string& name,
        std::string& header
    ) const;

    virtual bool has(const std::string& name) const = 0;
    virtual bool has_include_dirs(const std::string& name) const = 0;
    virtual const inc_dir_set& include_dirs(const std::string& name) const = 0;

    virtual void header_to_module(
        const std::string& name,
        const std::string& header,
        module_dep_info& module
    ) const = 0;

protected:
    const zap::env& env() const;
    const std::string& root() const;

    strings make_config_paths(
        const std::string& conf_dir,
        package_config_mode mode = package_config_mode::all_
    ) const;

    void set_config_paths(const std::string& conf_dir);

    const strings& config_paths() const;

private:
    void push_config_paths(strings& to, strings& from) const;

    const zap::env& e_;
    std::string root_;
    package_config_type type_;
    strings config_paths_;
};

}
