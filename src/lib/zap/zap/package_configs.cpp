#include <zap/package_configs.hpp>

namespace zap {

bool
strip_header(const inc_dir_set& inc_dirs, std::string& header)
{
    // INCDIR/boost/whatever.hpp --> boost/whatever.hpp
    bool relative = false;

    // Note: longest to shortest path
    for (const auto& inc_dir : zap::reverse(inc_dirs)) {
        if (header.starts_with(inc_dir)) {
            // Only one directory should match
            header.erase(0, inc_dir.size());
            relative = true;
            break;
        }
    }

    return relative;
}

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

void
package_configs::strip_header(
    const inc_dir_set& default_dirs,
    const std::string& name,
    std::string& header
) const
{
    if (has_include_dirs(name)) {
        zap::strip_header(include_dirs(name), header);
    } else {
        zap::strip_header(default_dirs, header);
    }
}

const toolchain&
package_configs::tc() const
{ return tc_; }

const std::string&
package_configs::root() const
{ return root_; }

}
