#pragma once

#include <memory>
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

using command_ptr = std::unique_ptr<command>;

template <typename Command, typename... Args>
command_ptr
new_command(Args&&... args)
{ return std::make_unique<Command>(std::forward<Args>(args)...); }

}
