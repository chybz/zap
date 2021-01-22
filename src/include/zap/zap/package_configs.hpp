#pragma once

#include <set>
#include <unordered_map>

#include <zap/toolchain.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/dep_info.hpp>
#include <zap/package_config_type.hpp>

namespace zap {

bool
strip_header(const inc_dir_set& inc_dirs, std::string& header);

class package_configs
{
public:
    package_configs(
        const toolchain& tc,
        const std::string& root,
        package_config_type type
    );

    virtual ~package_configs();

    package_config_type type() const;

    bool strip_header(const std::string& name, std::string& header) const;

    virtual bool has(const std::string& name) const = 0;
    virtual bool has_include_dirs(const std::string& name) const = 0;
    virtual const inc_dir_set& include_dirs(const std::string& name) const = 0;

    virtual void header_to_module(
        const std::string& name,
        const std::string& header,
        module_dep_info& module
    ) const = 0;

protected:
    const toolchain& tc() const;
    const std::string& root() const;

private:
    const toolchain& tc_;
    std::string root_;
    package_config_type type_;
};

}
