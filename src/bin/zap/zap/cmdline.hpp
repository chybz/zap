#pragma once

#include <string>

#include <zap/cmd.hpp>

namespace zap {

struct cmdline
{
    cmd c;
    bool exit = false;

    void run();
};

cmdline
parse(int ac, char** argv);

} // namespace zap
