#pragma once

#include <unordered_map>

#include <zap/types.hpp>

namespace zap {

struct resolve_info
{
    string_set to_install;
    string_set to_choose;
    string_set unresolved;
    std::unordered_map<std::string, string_set> ambiguous;
};

}
