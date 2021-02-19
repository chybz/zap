#include <zap/cmake/project.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

library&
project::get_library(const std::string& name)
{
    auto it = libs.find(name);

    if (it == libs.end()) {
        if (aliases.contains(name)) {
            it = libs.find(aliases.at(name));
        }
    }

    die_if(
        it == libs.end(),
        "no such library or alias: ", name
    );

    return it->second;
}

}
