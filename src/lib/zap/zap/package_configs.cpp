#include <zap/package_configs.hpp>

namespace zap {

package_configs::package_configs(const toolchain& tc, const std::string& root)
: tc_{tc},
root_(root)
{}

package_configs::~package_configs()
{}

const toolchain&
package_configs::tc() const
{ return tc_; }

const std::string&
package_configs::root() const
{ return root_; }

}
