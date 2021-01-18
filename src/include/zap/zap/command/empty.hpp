#pragma once

#include <zap/toolchain.hpp>

namespace zap::command {

struct empty
{
    void operator()(const toolchain& tc);
};

}
