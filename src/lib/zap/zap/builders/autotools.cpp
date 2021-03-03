#include <zap/builders/autotools.hpp>
#include <zap/utils.hpp>

namespace zap::builders {

autotools::autotools(const zap::env& e, const archive_info& ai)
: builder_base(e, ai)
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

            config.run(
                {
                    zap::cat("--prefix=", e_["root"]),
                    "--enable-shared"
                },
                e_.build_env()
            );
        }
    );
}

void
autotools::build() const
{
    make_.run(
        { "-C", build_dir_ },
        e_.build_env()
    );
}

const std::string&
autotools::install() const
{
    make_.run(
        {
            "-C", build_dir_,
            zap::cat("DESTDIR=", stage_dir_),
            "install"
        },
        e_.build_env()
    );

    return stage_dir_;
}

}
