#pragma once

#include <string>

#include <zap/command.hpp>
#include <zap/types.hpp>

namespace zap::commands {

enum class remote_cmd
{
    new_remote,
    delete_remote,
    ls_remote
};

struct remote_opts
{
    remote_cmd cmd;
    std::string id;
    std::string url;
    std::string type;
};

class remote : public zap::command
{
public:
    remote(const zap::env& e, const remote_opts& opts);
    virtual ~remote();

    void operator()() final;

private:
    void new_remote();
    void delete_remote();
    void ls_remote();

    remote_opts opts_;
};

}
