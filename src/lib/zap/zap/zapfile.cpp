#include <string>

#include <yaml-cpp/yaml.h>
#include <re2/re2.h>

#include <zap/zapfile.hpp>
#include <zap/utils.hpp>

namespace zap {

void
zapfile::load(const std::string& file)
{
    if (!file_exists(file)) {
        std::cout << "invalid file: " << file << std::endl;

        return;
    }

    YAML::Node c = YAML::LoadFile(file);

    if (c["depends"]) {
        const auto& deps = c["depends"];

        if (!deps.IsSequence()) {
            std::cout << "invalid depends" << std::endl;
            return;
        }

        re2::RE2 depexpr("(\\w+:)?(.+)@(.+)");
        std::string type;
        std::string path;
        std::string version;


        for (const auto& dep : deps) {
            std::cout << "new dep" << std::endl;
            std::string s;


            if (dep.IsScalar()) {
                s = dep.as<std::string>();
            } else if (dep.IsMap()) {
                if (dep.size() != 1) {
                    std::cout << "FUCK";
                }

                auto it = dep.begin();
                s = it->first.as<std::string>();
                std::cout << it->second << std::endl;
                const auto& opts = it->second;

                if (!opts.IsMap()) {
                    std::cout << "invalid dependency options";
                }
            } else {
                std::cout << "other" << std::endl;
            }

            if (!re2::RE2::FullMatch(s, depexpr, &type, &path, &version)) {
                std::cout << "invalid dependency: " << s << std::endl;
            } else {
                std::cout
                    << "dep type=" << type
                    << " path=" << path
                    << " version=" << version
                    << std::endl;
            }
        }
    }
}

}
