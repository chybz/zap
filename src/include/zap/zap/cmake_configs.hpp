#pragma once

#include <set>
#include <unordered_map>
#include <string_view>

#include <re2/re2.h>

#include <zap/toolchain.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>

namespace zap {

struct cmake_target
{
    std::string name;
    std::string module;
};

using cmake_targets = std::unordered_map<std::string, cmake_target>;

struct cmake_module
{
    using inc_dir_set = std::set<std::string>;

    std::string name;
    bool has_components = false;
    cmake_targets targets;
    inc_dir_set inc_dirs;
};

using cmake_modules = std::unordered_map<std::string, cmake_module>;

struct cmake_module_info
{
    std::string config;
    std::string name;
};

class cmake_configs
{
public:
    using inc_dir_set = std::set<std::string>;

    cmake_configs(const toolchain& tc, const std::string& root);
    virtual ~cmake_configs();

    const cmake_modules& modules() const;

    bool has(const std::string& module) const;
    bool has_include_dirs(const std::string& module) const;
    const inc_dir_set& include_dirs(const std::string& module) const;

private:
    void load_configs();

    cmake_module_info module_info(const std::string& path) const;
    bool is_module_config(const std::string& path) const;

    void scan_config(
        cmake_modules& modules,
        const std::string& module,
        const std::string& f
    );

    void find_targets(
        cmake_modules& modules,
        const std::string& module,
        const std::string& data
    );

    void write_cmake_file(
        const cmake_modules& modules,
        const std::string& module,
        const std::string& file
    );

    void get_properties(
        cmake_modules& modules,
        const std::string& module,
        const std::string& dir
    );

    void process_modules(cmake_modules& modules);

    bool clean_dir(std::string_view& dir) const;

    const toolchain& tc_;
    std::string root_;
    prog cmake_;
    cmake_modules modules_;
    re2::RE2 add_lib_re_;
    re2::RE2 target_name_re_;
    re2::RE2 inc_dirs_re_;
};

}
