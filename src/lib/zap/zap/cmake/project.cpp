#include <zap/cmake/project.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

void
project::clear()
{
    dir.clear();
    name.clear();
    libs.clear();
    map.clear();
    aliases.clear();
}

bool
project::has_library(const std::string& name) const
{ return map.contains(name); }

void
project::add_library(const std::string& name)
{
    library l{ name };

    auto pos = libs.size();

    libs.emplace_back(std::move(l));
    map[name] = pos;
}

void
project::add_library(const std::string& name, const std::string& target)
{
    // Per CMake spec, name must have been declared already
    map[target] = map[name];
}

library&
project::get_library(const std::string& name)
{ return libs[map[name]]; }

}
