#pragma once

#include <cstddef>

#include <zap/toolchain.hpp>

namespace zap::command {

struct build
{
    std::size_t cpus = 0;

    void operator()(const toolchain& tc);
};

}
