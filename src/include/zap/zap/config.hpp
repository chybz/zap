#pragma once

#include <string>

#include <toml++/toml.h>

namespace zap {

struct config
{
    std::string home;
    std::string project_dir;
    std::string build_dir;
    std::string archives_dir;
    std::string work_dir;
    std::string local_prefix;
    std::string empty_dir;
    std::string empty_source_file;
    std::string package_file;

    toml::table package_conf;

    config();
    virtual ~config();

    void load_package_conf();

    bool has(const std::string& name) const;
    std::string str(const std::string& name) const;
};

}
