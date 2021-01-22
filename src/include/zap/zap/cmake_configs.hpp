#pragma once

#include <set>
#include <unordered_map>
#include <string_view>

#include <re2/re2.h>

#include <zap/toolchain.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/package_configs.hpp>

namespace zap {

struct cmake_config_context
{
    zap::string_set component_modules;
    zap::string_set names;
    zap::string_set target_names;
    zap::string_set_map targets;
    zap::inc_dir_sets target_inc_dirs;
    zap::string_map libraries;
    zap::inc_dir_sets inc_dirs;
    zap::string_set_map deps;
    zap::string_set_map revdeps;

    void merge(cmake_config_context& other)
    {
        component_modules.merge(other.component_modules);
        names.merge(other.names);
        target_names.merge(other.target_names);
        targets.merge(other.targets);
        target_inc_dirs.merge(other.target_inc_dirs);
        libraries.merge(other.libraries);
        inc_dirs.merge(other.inc_dirs);

        for (auto& p : other.deps) {
            deps[p.first].merge(p.second);
        }

        for (auto& p : other.revdeps) {
            revdeps[p.first].merge(p.second);
        }
    }
};

struct cmake_module_info
{
    std::string config;
    std::string name;
};

struct cmake_module
{
    bool has_components = false;
    string_map targets;
    inc_dir_set inc_dirs;
};

using cmake_modules = std::unordered_map<std::string, cmake_module>;

class cmake_configs : public package_configs
{
public:
    cmake_configs(const toolchain& tc, const std::string& root);
    virtual ~cmake_configs();

    bool has(const std::string& module) const final;
    bool has_include_dirs(const std::string& module) const final;
    const inc_dir_set& include_dirs(const std::string& module) const final;

    void header_to_module(
        const std::string& name,
        const std::string& header,
        module_dep_info& module
    ) const final;

private:
    void load_configs();

    cmake_module_info module_info(const std::string& path) const;

    void scan_config(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& f,
        bool main_config = false
    );

    void find_targets(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void write_cmake_file(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& file
    );

    void get_properties(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& dir
    );

    void parse_target_dirs(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void parse_target_location(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void parse_target_libs(
        cmake_config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void process_modules(cmake_config_context& ctx);

    bool clean_dir(std::string_view& dir) const;

    prog cmake_;
    zap::string_set names_;
    inc_dir_sets inc_dirs_;
    cmake_modules modules_;
    string_map dir_to_target_;
    string_set unique_targets_;
    re2::RE2 add_lib_re_;
    re2::RE2 target_name_re_;
    re2::RE2 location_re_;
    re2::RE2 link_lib_re_;
    re2::RE2 inc_dirs_re_;
};

}
