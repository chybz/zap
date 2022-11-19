#pragma once

#include <zap/types.hpp>

namespace zap {

struct env_paths
{
    string_map sm;

    const std::string& operator[](const std::string& name) const;
};

}
