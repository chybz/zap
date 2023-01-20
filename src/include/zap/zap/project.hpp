#pragma once

#include <string>

#include <zap/target.hpp>
#include <zap/types.hpp>
#include <zap/graph.hpp>
#include <zap/dependency.hpp>

namespace zap {

struct project_langs
{
    bool c = true;
    bool cpp = true;
};

struct project
{
    std::string name;
    std::string version;

    project_langs langs;

    dependencies deps;

    bool sub_targets = false;

    targets bins;
    targets libs;
    targets mods;
    targets tsts;

    std::string root_dir;
    strings inc_dirs;

    graph g;

    void add_target(targets& ts, const std::string& tname, target& t);
};

}
