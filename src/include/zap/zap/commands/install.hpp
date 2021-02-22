#pragma once

#include <string>

#include <zap/command.hpp>

namespace zap::commands {

class install : public zap::command
{
public:
    install(const zap::env& e);
    virtual ~install();

    std::string target;

    void operator()() final;
};

}
