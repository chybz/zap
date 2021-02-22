#pragma once

#include <zap/env.hpp>

namespace zap {

class command
{
public:
    command(const zap::env& e);
    virtual ~command();

    virtual void operator()() = 0;

protected:
    const zap::env& env() const;

private:
    const zap::env& e_;
};

}
