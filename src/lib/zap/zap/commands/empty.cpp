#include <zap/commands/empty.hpp>

namespace zap::commands {

empty::empty(const zap::env& e)
: zap::command(e)
{}

empty::~empty()
{}

void
empty::operator()()
{}

}
