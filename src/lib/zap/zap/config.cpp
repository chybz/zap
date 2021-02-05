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

void
config::load_package_conf()
{
    if (file_exists(package_file)) {
        return;
    }

    package_conf = toml::parse_file(package_file);
}

bool
config::has(const std::string& name) const
{
    if (package_conf.contains(name)) {
        return true;
    }

    return false;
}

std::string
config::str(const std::string& name) const
{ return package_conf[name].value_or<std::string>({}); }

}
