#include <zap/sys_db_types.hpp>

namespace zap {

bool
sys_db_info::contains(const std::string& name) const
{ return vm.contains(name); }

void
sys_db_info::erase(const std::string& name)
{ vm.erase(name); }

std::string&
sys_db_info::operator[](const std::string& name)
{ return vm[name]; }

const std::string&
sys_db_info::operator[](const std::string& name) const
{ return vm.at(name); }

}
