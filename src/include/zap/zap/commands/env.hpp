#pragma once

#include <zap/command.hpp>

namespace zap::commands {

enum class env_cmd
{
    new_env,
    delete_env,
    ls_env,
    ls_pkgs
};

struct env_opts
{
    env_cmd cmd;
    std::string name;
    std::string directory;
};

class env : public zap::command
{
public:
    env(const zap::env& e, const env_opts& opts);
    virtual ~env();

    void operator()() final;

private:
    void new_env();
    void delete_env();
    void ls_env();
    void ls_pkgs();

    env_opts opts_;
};

}
