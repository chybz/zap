#include <zap/builders/cmake.hpp>
#include <zap/utils.hpp>

namespace zap::builders {

cmake::cmake(const zap::env& e, const archive_info& ai)
: builder_base(e, ai)
{
    cmake_.cmd = zap::find_cmd("cmake");
    make_.cmd = zap::find_cmd("make");
    build_dir_ = cat_dir(ai.dir, "build");
    stage_dir_ = cat_dir(ai.dir, "stage");
}

cmake::~cmake()
{}

void
cmake::configure() const
{
    zap::mkpath(build_dir_);

    cmake_.run({
        zap::cat("-DCMAKE_INSTALL_PREFIX=", e_["root"]),
        "-S", ai_.source_dir, "-B", build_dir_
    });
}

void
cmake::build() const
{
    cmake_.run({
       "--build", build_dir_
    });
}

void
cmake::install() const
{
    make_.run({
        "-C", build_dir_,
        zap::cat("DESTDIR=", stage_dir_),
        "install"
    });
}

}
