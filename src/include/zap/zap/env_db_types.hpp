#pragma once

#include <zap/types.hpp>

namespace zap {

struct env_db_pkg
{
    std::string name;
    std::string version;
};

struct env_db_pkg_file
{
    std::string pkg;
    std::string file;
};

struct env_db_pkg_files
{
    std::string pkg;
    string_set files;
};

}
