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
    std::string meta_file;

    toml::table meta;

    config();
    virtual ~config();

    bool has_meta() const;
    void load_meta();

    bool has(const std::string& name) const;
    std::string str(const std::string& name) const;
};

}
