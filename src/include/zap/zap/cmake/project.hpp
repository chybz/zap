#pragma once

#include <utility>
#include <string>
#include <vector>
#include <unordered_map>

#include <zap/types.hpp>
#include <zap/link_type.hpp>

namespace zap::cmake {

struct library
{
    std::string name;
    link_type link;
    std::string alias;
    std::string source_interface_dir;
    std::string installed_interface_dir;
    zap::string_set headers;

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

    void clear();

    void clean_libraries(const std::string& inst_dir);

    bool has_library(const std::string& name) const;

    void add_library(const std::string& name, zap::link_type lt);
    void add_alias(const std::string& name, const std::string& target);

    library& get_library(const std::string& name);

    void add_header(const std::string& name, const std::string& header);
    void add_headers(const std::string& name, const zap::string_set& hs);

    void set_interface_dirs(
        const std::string& name,
        const std::string& source,
        const std::string& installed
    );

    void reindex();
};

}
