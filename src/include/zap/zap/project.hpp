#pragma once

#include <string>
#include <map>
#include <set>

#include <zap/target.hpp>
#include <zap/types.hpp>
#include <zap/graph.hpp>

namespace zap {

struct project
{
    std::string name;
    std::string version;

    targets bins;
    targets libs;
    targets mods;
    targets tsts;

    string_set_map cmake_components;
    string_set cmake_modules;
    string_set pkg_configs;
    string_set raw_libs;

    std::string root_dir;
    strings inc_dirs;

    graph g;
    strings ordered_deps;

    void finalize();

    void build_deps_graph(const targets& ts);
    void build_deps_graph(const target& t);
    void build_deps_graph(const target& t, const target_deps& deps);

    void order_deps();
    void order_deps(targets& ts);
    void order_deps(target& t, target_deps& libs);

    void normalize_deps();
    void normalize_deps(targets& ts);
    void normalize_deps(target& t, string_set& libs);

    bool dep_has_dep(const target& t, const std::string& dep) const;
};

}
