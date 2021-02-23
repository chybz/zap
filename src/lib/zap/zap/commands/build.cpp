#include <zap/commands/build.hpp>

namespace zap::commands {

build::build(const zap::env& e, const build_opts& opts)
: zap::command(e),
opts_(opts)
{}

build::~build()
{}

void
build::operator()()
{}

}
