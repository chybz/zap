#pragma once

#include <string>

#include <zap/command.hpp>
#include <zap/types.hpp>

namespace zap::commands {

struct analyze_opts
{
    std::string directory;
};

class analyze : public zap::command
{
public:
    analyze(const zap::env& e, const analyze_opts& opts);
    virtual ~analyze();

    void operator()() final;

private:
    analyze_opts opts_;
};

}
