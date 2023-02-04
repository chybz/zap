#pragma once

#include <zap/dependency.hpp>
#include <yaml-cpp/yaml.h>

namespace zap {

struct zapfile
{
    std::string name;
    std::string version;
    dependencies deps;

    void load(const std::string& file = "Zapfile");

    void load_deps(const YAML::Node& c);

    void set_remote(
        const std::string& type,
        const std::string& path,
        const std::string& version
    );

    void load_strings(
        const YAML::Node& n,
        const std::string& key,
        strings_map& m
    );
};

}
