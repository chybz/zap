#include <zap/env_paths.hpp>

namespace zap {

const std::string&
env_paths::operator[](const std::string& name) const
{ return sm.at(name); }

}
