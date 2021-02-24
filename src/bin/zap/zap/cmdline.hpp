#pragma once

#include <string>

#include <zap/env.hpp>
#include <zap/command.hpp>

namespace zap {

struct cmdline
{
    command_ptr cp;
    bool exit = false;

    void run();
};

cmdline
parse(const zap::env& e, int ac, char** argv);

} // namespace zap
