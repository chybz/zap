#pragma once

#include <string>

#include <zap/target.hpp>
#include <zap/types.hpp>

namespace zap {

struct project
{
    std::string name;
    std::string version;

    targets bins;
    targets libs;
    targets mods;
    targets tsts;

    std::string root_dir;
    strings inc_dirs;
};

}
