#include <zap/project.hpp>

namespace zap {

void
project::finalize()
{
    //normalize_deps();
    order_deps();
}

void
project::order_deps()
{
    g.clear();

    build_deps_graph(libs);
    build_deps_graph(bins);
    build_deps_graph(mods);
    build_deps_graph(tsts);

    ordered_deps = g.ordered();

    order_deps(libs);
    order_deps(bins);
    order_deps(mods);
    order_deps(tsts);
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
    for (const auto& d : ordered_deps) {
        if (deps.project_libs.contains(d) || deps.libs.contains(d)) {
            deps.ordered_libs.emplace_back(d);
        }
    }
}

void
project::normalize_deps()
{
    normalize_deps(libs);
    normalize_deps(bins);
    normalize_deps(mods);
    normalize_deps(tsts);
}

void
project::normalize_deps(targets& ts)
{
    for (auto& p : ts) {
        auto& t = p.second;
        normalize_deps(t, t.public_deps.project_libs);
        normalize_deps(t, t.public_deps.libs);
        normalize_deps(t, t.private_deps.project_libs);
        normalize_deps(t, t.private_deps.libs);
    }
}

void
project::normalize_deps(target& t, string_set& libs)
{
    for (auto it = libs.begin(); it != libs.end(); ) {
        if (dep_has_dep(t, *it)) {
            it = libs.erase(it);
        } else {
            ++it;
        }
    }
}

bool
project::dep_has_dep(const target& t, const std::string& dep) const
{
    for (const auto& l : t.public_deps.project_libs) {
        if (libs.at(l).has_public_dep(dep)) {
            return true;
        }
    }

    for (const auto& l : t.private_deps.project_libs) {
        if (libs.at(l).has_public_dep(dep)) {
            return true;
        }
    }

    return false;
}

}
