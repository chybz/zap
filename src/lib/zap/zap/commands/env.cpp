#include <zap/commands/env.hpp>

namespace zap::commands {

env::env(const zap::env& e, const env_opts& opts)
: zap::command(e),
opts_(opts)
{}

env::~env()
{}

void
env::operator()()
{
    switch (opts_.cmd) {
        case env_cmd::new_env:
        new_env();
        break;
        case env_cmd::delete_env:
        delete_env();
        break;
        case env_cmd::ls_env:
        ls_env();
        break;
    }
}

void
env::new_env()
{
    command::env().sys_db().new_env(opts_.name, opts_.directory);
}

void
env::delete_env()
{}

void
env::ls_env()
{}

}
