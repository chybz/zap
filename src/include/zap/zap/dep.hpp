#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>

#include <zap/files.hpp>
#include <zap/types.hpp>

namespace zap {

using dep_set = std::unordered_set<std::string>;

struct dep
{
    std::string name;
    files headers;
    strings libs;
};

using deps = std::vector<dep>;
using dep_map = std::unordered_map<std::string, dep>;

}
