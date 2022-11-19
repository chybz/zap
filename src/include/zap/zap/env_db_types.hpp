#pragma once

#include <vector>

#include <zap/types.hpp>

namespace zap {

struct env_db_pkg
{
    std::string name;
    std::string version;
};

using env_db_pkgs = std::vector<env_db_pkg>;

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

struct env_db_archive
{
    std::string url;
    std::string file;
};

}
