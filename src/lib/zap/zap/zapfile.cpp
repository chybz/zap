#include <yaml-cpp/yaml.h>

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

        for (const auto& dep : deps) {
            std::cout << "new dep" << std::endl;

            if (dep.IsScalar()) {
                std::cout << "scalar" << std::endl;
            } else if (dep.IsMap()) {
                std::cout << "map" << std::endl;
            } else {
                std::cout << "other" << std::endl;
            }
        }
    }
}

}
