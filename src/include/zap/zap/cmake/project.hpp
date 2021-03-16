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

    void add_header(const std::string& header);
    void add_headers(const zap::string_set& hs);
    void clean_headers(const std::string& inst_dir);
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

    void clean_libraries(const std::string& inst_dir);

    bool has_library(const std::string& name) const;

    void add_library(const std::string& name);
    void add_alias(const std::string& name, const std::string& target);

    library& get_library(const std::string& name);

    void add_header(const std::string& name, const std::string& header);
    void add_headers(const std::string& name, const zap::string_set& hs);
    void set_interface_dir(const std::string& name, const std::string& dir);
};

}
