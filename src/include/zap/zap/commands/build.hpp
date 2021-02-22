#pragma once

#include <cstddef>

#include <zap/command.hpp>

namespace zap::commands {

class build : public zap::command
{
public:
    build(const zap::env& e);
    virtual ~build();

    void operator()() final;
};

}
