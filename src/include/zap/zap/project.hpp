#pragma once

#include <string>
#include <map>
#include <set>

#include <zap/target.hpp>
#include <zap/types.hpp>

namespace zap {

struct project
{
    using mods_set = std::set<std::string>;
    using comps_map = std::map<std::string, mods_set>;

    std::string name;
    std::string version;

    targets bins;
    targets libs;
    targets mods;
    targets tsts;

    comps_map cmake_components;
    mods_set cmake_modules;
    mods_set pkg_configs;

    std::string root_dir;
    strings inc_dirs;
};

}
