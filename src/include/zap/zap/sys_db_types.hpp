#pragma once

#include <unordered_map>

#include <zap/types.hpp>

namespace zap {

struct sys_db_info_val
{
    std::string name;
    std::string value;
};

struct sys_db_info
{
    using val_map = std::unordered_map<std::string, std::string>;

    bool contains(const std::string& name) const;
    void erase(const std::string& name);

    std::string& operator[](const std::string& name);
    const std::string& operator[](const std::string& name) const;

    val_map vm;
};

struct sys_db_env
{
    std::string name;
    std::string root;
};

using sys_db_envs = std::vector<sys_db_env>;

struct sys_db_remote
{
    std::string id;
    std::string url;
    std::string type;
};

using sys_db_remotes = std::unordered_map<std::string, sys_db_remote>;

}
