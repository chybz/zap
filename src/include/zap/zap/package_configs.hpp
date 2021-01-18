#pragma once

#include <set>
#include <unordered_map>

#include <zap/toolchain.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>

namespace zap {

class package_configs
{
public:
    package_configs(const toolchain& tc, const std::string& root);
    virtual ~package_configs();

    virtual bool has(const std::string& name) const = 0;
    virtual bool has_include_dirs(const std::string& name) const = 0;
    virtual const inc_dir_set& include_dirs(const std::string& name) const = 0;

protected:
    const toolchain& tc() const;
    const std::string& root() const;

private:
    const toolchain& tc_;
    std::string root_;
};

}
