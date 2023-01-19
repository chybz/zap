#pragma once

#include <zap/toolchain_type.hpp>
#include <zap/prog.hpp>
#include <zap/utils.hpp>

namespace zap {

struct toolchain_info
{
    prog cxx;
    prog cc;
    prog nm;
    prog ldd;
    prog fetcher;
    prog compiler_launcher;
    toolchain_type type = toolchain_type::unknown;
    std::string version;
};

void
detect_toolchain(toolchain_info& ti);

}
