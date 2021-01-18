#pragma once

#include <string>

#include <zap/toolchain.hpp>

namespace zap::command {

struct install
{
    std::string target;

    void operator()(const toolchain& tc);
};

}
