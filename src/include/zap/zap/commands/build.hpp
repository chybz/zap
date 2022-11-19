#pragma once

#include <zap/command.hpp>

namespace zap::commands {

struct build_opts
{
    std::size_t cpus = 0;
};

class build : public zap::command
{
public:
    build(const zap::env& e, const build_opts& opts);
    virtual ~build();

    void operator()() final;

private:
    build_opts opts_;
};

}
