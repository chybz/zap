#pragma once

#include <set>
#include <unordered_map>
#include <string_view>

#include <re2/re2.h>

#include <zap/env.hpp>
#include <zap/prog.hpp>
#include <zap/types.hpp>
#include <zap/inc_dirs.hpp>
#include <zap/package_configs.hpp>
#include <zap/frameworks.hpp>

namespace zap::cmake {

struct config_context
{
    zap::string_set component_modules;
    zap::string_set names;
    zap::string_set target_names;
    zap::string_set_map config_targets;
    zap::inc_dir_sets target_inc_dirs;
    zap::string_map libraries;
    zap::inc_dir_sets inc_dirs;
    zap::string_set_map deps;
    zap::string_set_map revdeps;

    void merge(config_context& other);
    void clear_locals();
};

struct module_info
{
    std::string config;
    std::string name;
};

struct module
{
    bool has_components = false;
    zap::string_map targets;
    zap::inc_dir_set inc_dirs;
};

using modules = std::unordered_map<std::string, module>;

class configs : public zap::package_configs
{
public:
    configs(const zap::env& e, const std::string& root);
    virtual ~configs();

    bool has(const std::string& module) const final;
    bool has_include_dirs(const std::string& module) const final;
    const zap::inc_dir_set& include_dirs(const std::string& module) const final;

    void header_to_module(
        const std::string& name,
        const std::string& header,
        zap::module_dep_info& module
    ) const final;

private:
    bool header_to_module_specific(
        const std::string& name,
        const std::string& header,
        zap::module_dep_info& module
    );

    bool boost_header_to_module(
        const std::string& name,
        const std::string& header,
        zap::module_dep_info& module
    );

    void load_configs();

    zap::cmake::module_info module_info(const std::string& path) const;

    void scan_config(
        config_context& ctx,
        const std::string& module,
        const std::string& f,
        bool main_config = false
    );

    void find_targets(
        config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void write_file(
        config_context& ctx,
        const std::string& module,
        const std::string& file
    );

    void get_properties(
        config_context& ctx,
        const std::string& module,
        const std::string& dir
    );

    void parse_target_dirs(
        config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void parse_target_location(
        config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    void parse_target_libs(
        config_context& ctx,
        const std::string& module,
        const std::string& data
    );

    template <typename Callable>
    void parse_target_var(
        const std::string& data,
        const re2::RE2& re,
        Callable&& cb
    );

    template <typename Callable>
    void parse_target_list(
        const std::string& data,
        const re2::RE2& re,
        Callable&& cb
    );

    void process_modules(config_context& ctx);

    bool clean_dir(std::string_view& dir) const;

    std::string root_;
    prog cmake_;
    zap::string_set names_;
    config_context data_;
    re2::RE2 add_lib_re_;
    re2::RE2 target_name_re_;
    re2::RE2 location_re_;
    re2::RE2 link_lib_re_;
    re2::RE2 inc_dirs_re_;
    zap::frameworks frameworks_;
};

}
