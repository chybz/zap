#include <zap/cmake/project.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

///////////////////////////////////////////////////////////////////////////////
//
// Library
//
///////////////////////////////////////////////////////////////////////////////
void
library::add_header(const std::string& header)
{ headers.insert(header); }

void
library::add_headers(const zap::string_set& hs)
{ headers.insert(hs.begin(), hs.end()); }

void
library::clean_headers(const std::string& inst_dir)
{
    for (auto it = headers.begin(), end = headers.end(); it != end; ) {

    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Project
//
///////////////////////////////////////////////////////////////////////////////
void
project::clear()
{
    dir.clear();
    name.clear();
    libs.clear();
    map.clear();
    aliases.clear();
}

void
project::clean_libraries(const std::string& inst_dir)
{
    for (auto it = libs.begin(), end = libs.end(); it != end; ) {
        auto& lib = *it;

        lib.clean_headers(inst_dir);

        if (lib.headers.empty()) {
            map.erase(lib.name);
            map.erase(lib.alias);

            it = libs.erase(it);
        } else {
            ++it;
        }
    }
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
project::add_alias(const std::string& name, const std::string& target)
{
    if (!has_library(target)) {
        return;
    }

    // Per CMake spec, target must have been declared already
    auto pos = map[target];

    libs[pos].alias = name;
    map[name] = pos;
}

library&
project::get_library(const std::string& name)
{ return libs[map[name]]; }

void
project::add_header(const std::string& name, const std::string& header)
{
    if (!has_library(name)) {
        return;
    }

    get_library(name).add_header(header);
}

void
project::add_headers(const std::string& name, const zap::string_set& hs)
{
    if (!has_library(name)) {
        return;
    }

    get_library(name).add_headers(hs);
}

void
project::set_interface_dir(const std::string& name, const std::string& dir)
{
    if (!has_library(name)) {
        return;
    }

    get_library(name).interface_dir = dir;
}

}
