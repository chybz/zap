#include <sstream>

#include <zap/dependency.hpp>

namespace zap {

std::string
to_string(dependency_type dt)
{
    static const std::unordered_map<dependency_type, std::string> dmap = {
        { dependency_type::github, "github" },
        { dependency_type::gitlab, "gitlab" }
    };

    return dmap.at(dt);
}

std::string
dependency::to_string() const
{
    std::ostringstream oss;

    oss
        << "type: " << zap::to_string(type)
        << " id:" << owner << "/" << repo
        << " version: " << version
        ;

    return oss.str();
}

}
