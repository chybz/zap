#include <iterator>

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
    const zap::env& e,
    const std::string& root,
    package_config_type type
)
: e_{e},
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

const zap::env&
package_configs::env() const
{ return e_; }

const std::string&
package_configs::root() const
{ return root_; }

strings
package_configs::make_config_paths(
    const std::string& conf_dir,
    package_config_mode mode
) const
{
    strings paths;
    const auto& tc = e_.toolchain();

    // conf_dir is something like: pkgconfig or cmake
    if (root_ != e_.root()) {
        auto stage_base = zap::cat(root_, e_.root());

        auto staging =
            directory_exists(stage_base)
            &&
            (
                mode == package_config_mode::all_
                ||
                mode == package_config_mode::private_
            );

        if (staging) {
            // We are loading from a staging directory
            auto stage_conf_dirs = tc.make_arch_conf_dirs(
                stage_base, conf_dir
            );

            push_config_paths(paths, stage_conf_dirs);
        } else {
            // Unrelated...
            auto conf_dirs = tc.make_arch_conf_dirs(root_, conf_dir);

            push_config_paths(paths, conf_dirs);
        }
    }

    if (
        mode == package_config_mode::all_
        ||
        mode == package_config_mode::public_
    ) {
        auto env_conf_dirs = tc.make_arch_conf_dirs(e_.root(), conf_dir);
        push_config_paths(paths, env_conf_dirs);
    }

    return paths;
}

void
package_configs::set_config_paths(const std::string& conf_dir)
{
    auto paths = make_config_paths(conf_dir);

    config_paths_ = std::move(paths);
}

const strings&
package_configs::config_paths() const
{ return config_paths_; }

void
package_configs::push_config_paths(strings& to, strings& from) const
{
    to.insert(
        to.end(),
        std::make_move_iterator(from.begin()),
        std::make_move_iterator(from.end())
    );
}

}
