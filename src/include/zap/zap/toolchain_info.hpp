#pragma once

#include <zap/toolchain_type.hpp>
#include <zap/prog.hpp>
#include <zap/utils.hpp>
#include <zap/link_type.hpp>

namespace zap {

struct toolchain_info
{
    prog cxx;
    prog cc;
    prog nm;
    prog ldd;
    prog fetcher;
    toolchain_type type = toolchain_type::unknown;
    std::string version;
    link_type link = link_type::link_shared;

    bool link_shared() const;
    bool link_static() const;
};

void
detect_toolchain(toolchain_info& ti);

}
