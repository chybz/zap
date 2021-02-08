#include <iostream>

#include <zap/project.hpp>

namespace zap {

void
project::add_target(targets& ts, const std::string& tname, target& t)
{ ts.try_emplace(tname, std::move(t)); }

bool
project::has_pkg_configs() const
{ return !pkg_configs.empty(); }

bool
project::has_cmake_components() const
{ return !cmake_components.empty(); }

bool
project::has_cmake_modules() const
{ return !cmake_modules.empty(); }

bool
project::has_raw_libs() const
{ return !raw_libs.empty(); }

void
project::finalize()
{
    build_deps_graph();
    order_deps();
}

void
project::build_deps_graph()
{
    g.clear();

    build_deps_graph(libs);
    build_deps_graph(bins);
    build_deps_graph(mods);
    build_deps_graph(tsts);

    g.build();
}

void
project::build_deps_graph(const targets& ts)
{
    for (const auto& p : ts) {
        build_deps_graph(p.second);
    }
}

void
project::build_deps_graph(const target& t)
{
    build_deps_graph(t, t.public_deps);
    build_deps_graph(t, t.private_deps);
}

void
project::build_deps_graph(const target& t, const target_deps& deps)
{
    if (t.is_lib()) {
        g.add_node(t.name);
        g.add_nodes(deps.project_libs, t.name);
        g.add_nodes(deps.libs, t.name);
    } else {
        g.add_nodes(deps.project_libs);
        g.add_nodes(deps.libs);
    }
}

void
project::order_deps()
{
    order_deps(libs);
    order_deps(bins);
    order_deps(mods);
    order_deps(tsts);
}

void
project::order_deps(targets& ts)
{
    for (auto& p : ts) {
        auto& t = p.second;
        order_deps(t, t.public_deps);
        order_deps(t, t.private_deps);
    }
}

void
project::order_deps(target& t, target_deps& deps)
{
    for (const auto& d : g.ordered()) {
        if (deps.project_libs.contains(d) || deps.libs.contains(d)) {
            deps.ordered_libs.emplace_back(d);
        }
    }
}

}
