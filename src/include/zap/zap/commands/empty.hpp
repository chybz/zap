#pragma once

#include <zap/command.hpp>

namespace zap::commands {

class empty : public zap::command
{
public:
    empty(const zap::env& e);
    virtual ~empty();

    void operator()() final;
};

}
