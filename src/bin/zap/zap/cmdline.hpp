#pragma once

#include <string>

#include <zap/env.hpp>
#include <zap/command.hpp>

namespace zap {

struct cmdline
{
    std::string env_name;
    env_ptr ep;
    command_ptr cp;
    bool exit = false;

    const zap::env& env() const;

    void run();
};

cmdline
parse(int ac, char** argv);

} // namespace zap
