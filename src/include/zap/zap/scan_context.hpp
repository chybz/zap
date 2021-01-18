#pragma once

#include <vector>

#include <zap/types.hpp>

namespace zap {

struct scan_context
{
    string_set deps;

    void merge(scan_context& other);
};

using scan_contexts = std::vector<scan_context>;

}
