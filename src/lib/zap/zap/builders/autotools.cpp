#include <zap/builders/autotools.hpp>
#include <zap/utils.hpp>

namespace zap::builders {

autotools::autotools(
    const zap::env& e,
    const archive_info& ai,
    const zap::strings& args
)
: builder_base(e, ai, args)
{
    make_.cmd = zap::find_cmd("make");
    build_dir_ = cat_dir(ai.dir, "build");
    stage_dir_ = cat_dir(ai.dir, "stage");
}

autotools::~autotools()
{}

void
autotools::configure() const
{
    zap::mkpath(build_dir_);

    zap::call_in_directory(
        build_dir_,
        [&] {
            zap::prog config{ zap::cat_file(ai_.source_dir, "configure") };

            config.run({
                .args = {
                    zap::cat("--prefix=", e_["root"]),
                    "--enable-shared"
                },
                .env = e_.build_env()
            });
        }
    );
}

void
autotools::build() const
{
    make_.run({
        .args = { "-C", build_dir_ },
        .env = e_.build_env()
    });
}

void
autotools::install(zap::package::manifest& pm) const
{
    make_.run({
        .args = {
            "-C", build_dir_,
            zap::cat("DESTDIR=", stage_dir_),
            "install"
        },
        .env = e_.build_env()
    });
}

}
