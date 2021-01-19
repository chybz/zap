#include <zap/package_configs.hpp>

namespace zap {

package_configs::package_configs(
    const toolchain& tc,
    const std::string& root,
    package_config_type type
)
: tc_{tc},
root_(root),
type_(type)
{}

package_configs::~package_configs()
{}

package_config_type
package_configs::type() const
{ return type_; }

const toolchain&
package_configs::tc() const
{ return tc_; }

const std::string&
package_configs::root() const
{ return root_; }

}
