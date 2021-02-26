#pragma once

#include <zap/types.hpp>

namespace zap {

struct pkg
{
    std::string name;
    std::string version;
};

struct pkg_file
{
    std::string pkg;
    std::string file;
};

struct pkg_files
{
    std::string pkg;
    string_set files;
};

}
