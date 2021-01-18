#include <fstream>

#include <zap/config.hpp>
#include <zap/utils.hpp>

namespace zap {

config::config()
{
    home = zap::cat_dir(zap::home_directory(), ".zap");
}

config::~config()
{}

bool
config::has_meta() const
{ return file_exists(meta_file); }

void
config::load_meta()
{
    if (!has_meta()) {
        return;
    }

    meta = toml::parse_file(meta_file);
}

bool
config::has(const std::string& name) const
{
    if (meta.contains(name)) {
        return true;
    }

    return false;
}

std::string
config::str(const std::string& name) const
{ return meta[name].value_or<std::string>({}); }

}
