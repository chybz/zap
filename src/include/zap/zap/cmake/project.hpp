#pragma once

#include <string>
#include <unordered_map>

#include <zap/types.hpp>

namespace zap::cmake {

struct library
{
    std::string name;
    std::string alias;
    zap::string_set headers;
};

using library_map = std::unordered_map<std::string, library>;

struct project
{
    std::string dir;
    std::string name;
    library_map libs;
    zap::string_map aliases;

    library& get_library(const std::string& name);
};

}
