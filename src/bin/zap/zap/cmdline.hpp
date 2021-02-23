#pragma once

#include <string>

#include <zap/env.hpp>
#include <zap/cmd.hpp>

namespace zap {

struct cmdline
{
    cmd c;
    bool exit = false;

    void run();
};

cmdline
parse(const zap::env& e, int ac, char** argv);

} // namespace zap
