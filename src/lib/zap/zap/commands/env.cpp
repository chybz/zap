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
{}

}
