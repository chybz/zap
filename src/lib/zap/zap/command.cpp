#include <zap/command.hpp>

namespace zap {

command::command(const zap::env& e)
: e_(e)
{}

command::~command()
{}

const zap::env&
command::env() const
{ return e_; }

}
