#include <iostream>

#include <zap/project.hpp>

namespace zap {

void
project::add_target(targets& ts, const std::string& tname, target& t)
{ ts.try_emplace(tname, std::move(t)); }

}
