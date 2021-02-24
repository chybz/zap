#pragma once

#include <string>

#include <zap/command.hpp>
#include <zap/types.hpp>

namespace zap::commands {

struct install_opts
{
    std::string file;
    std::string target;
    zap::strings args;
};

class install : public zap::command
{
public:
    install(const zap::env& e, const install_opts& opts);
    virtual ~install();

    std::string target;

    void operator()() final;

private:
    install_opts opts_;
};

}
