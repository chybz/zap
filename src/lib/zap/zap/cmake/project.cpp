#include <zap/cmake/project.hpp>
#include <zap/log.hpp>
#include <zap/utils.hpp>

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
    std::erase_if(
        headers,
        [&](const auto& item) {
            return
                !zap::file_exists(
                    zap::cat_file(inst_dir, installed_interface_dir, item)
                );
        }
    );
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
}

void
project::clean_libraries(const std::string& inst_dir)
{
    for (auto it = libs.begin(); it != libs.end(); ) {
        auto& lib = *it;

        lib.clean_headers(inst_dir);

        if (lib.headers.empty()) {
            it = libs.erase(it);
        } else {
            ++it;
        }
    }

    reindex();
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
project::set_interface_dirs(
    const std::string& name,
    const std::string& source,
    const std::string& installed
)
{
    if (!has_library(name)) {
        return;
    }

    auto& l = get_library(name);

    l.source_interface_dir = source;
    l.installed_interface_dir = installed;
}

void
project::reindex()
{
    map.clear();

    for (std::size_t i = 0; i < libs.size(); ++i) {
        const auto& l = libs[i];

        map[l.name] = i;

        if (!l.alias.empty()) {
            map[l.alias] = i;
        }
    }
}

}
