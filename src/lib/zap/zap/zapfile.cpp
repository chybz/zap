#include <string>

#include <re2/re2.h>

#include <zap/zapfile.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap {

void
zapfile::load(const std::string& file)
{
    die_unless(file_exists(file), "invalid file: ", file);

    YAML::Node c = YAML::LoadFile(file);

    load_deps(c);
}

void
zapfile::load_deps(const YAML::Node& c)
{
    if (!c["depends"]) {
        return;
    }

    const auto& deps = c["depends"];

    die_unless(deps.IsSequence(), "depends is not a sequence");

    re2::RE2 depexpr("(?:(\\w+):)?(.+)@(.+)");
    re2::RE2 urlexpr("(\\w+)://.+");

    std::string type;
    std::string path;
    std::string version;
    std::string scheme;

    for (const auto& dep : deps) {
        std::string s;

        if (dep.IsScalar()) {
            s = dep.as<std::string>();
        } else if (dep.IsMap()) {
            die_if(dep.size() != 1, "dependency map size is not 1: ", dep);

            auto it = dep.begin();
            s = it->first.as<std::string>();
            auto opts = it->second;

            die_unless(
                opts.IsMap(), "dependency options is not a map: ",
                opts
            );
        } else {
            die("dependency is not a scalar or map: ", dep);
        }

        if (re2::RE2::FullMatch(s, depexpr, &type, &path, &version)) {
            std::cout
                << "dep type=" << type
                << " path=" << path
                << " version=" << version
                << std::endl;
        } else if (re2::RE2::FullMatch(s, urlexpr, &scheme)) {
            std::cout
                << "dep direct URL scheme=" << scheme
                << std::endl;
        } else {
            die("invalid dependency: ", s);
        }
    }
}

}
