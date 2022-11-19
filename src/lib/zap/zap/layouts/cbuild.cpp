#include <zap/layouts/cbuild.hpp>

namespace zap::layouts {

static zap::string_map cbuild_sub_dirs = {
    { "src", "sources" }, // base directory for all sub directories below
    { "inc", "include" },
    { "bin", "binaries" },
    { "lib", "libraries" },
    { "mod", "plugins" },
    { "tst", "tests" }
};

cbuild::cbuild(const std::string& project_dir)
: app("cbuild project", project_dir, cbuild_sub_dirs)
{}

cbuild::~cbuild()
{}

}
