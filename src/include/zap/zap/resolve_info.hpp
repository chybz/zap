#pragma once

#include <unordered_map>

#include <zap/types.hpp>

namespace zap {

struct resolve_info
{
    string_set used;
    string_set to_install;
    string_set to_choose;
    string_set unresolved_headers;
    std::unordered_map<std::string, string_set> ambiguous_headers;

    bool empty() const;
};

}
