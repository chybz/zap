#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <zap/types.hpp>

namespace zap {

enum class dep_status
{
    not_found,
    found,
    ambiguous
};

struct dep_info
{
    dep_status status = dep_status::not_found;
    std::string pkg;
    std::string pc;
    std::string file;
    string_set pkg_candidates;

    bool not_found() const;
    bool found() const;
    bool ambiguous() const;

    bool has_pkg() const;
    bool has_pkg_candidates() const;
};

using dep_infos = std::vector<dep_info>;
using dep_info_map = std::unordered_map<std::string, dep_info>;

void
normalize_deps(dep_info_map& m, const string_set& installed);

}
