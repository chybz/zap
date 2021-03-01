#pragma once

#include <zap/types.hpp>

namespace zap {

struct sys_db_info
{
    std::string default_env;
};

struct sys_db_env
{
    std::string name;
    std::string root;
};

using sys_db_envs = std::vector<sys_db_env>;

}
