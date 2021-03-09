#pragma once

#include <utility>
#include <string>
#include <vector>
#include <unordered_map>

#include <zap/types.hpp>

namespace zap::cmake {

struct library
{
    std::string name;
    std::string alias;
    std::string interface_dir;
    string_set headers;
};

using libraries = std::vector<library>;
using library_map = std::unordered_map<std::string, std::size_t>;

struct project
{
    std::string dir;
    std::string name;
    libraries libs;
    library_map map;
    zap::string_map aliases;

    void clear();

    bool has_library(const std::string& name) const;

    void add_library(const std::string& name);
    void add_alias(const std::string& name, const std::string& target);

    library& get_library(const std::string& name);
};

}
