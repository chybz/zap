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
};

}
