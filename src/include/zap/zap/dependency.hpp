#pragma once

#include <vector>

#include <zap/types.hpp>

namespace zap {

enum class dependency_type
{
    github,
    gitlab
};

std::string
to_string(dependency_type dt);

struct dependency
{
    dependency_type type;
    std::string owner;
    std::string repo;
    std::string version;
    strings_map opts;

    std::string to_string() const;
};

using dependencies = std::vector<dependency>;

}
