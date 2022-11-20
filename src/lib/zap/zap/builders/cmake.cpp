#include <thread>

#include <zap/builders/cmake.hpp>
#include <zap/utils.hpp>
#include <zap/cmake/trace_parser.hpp>
#include <zap/cmake/toolchain_file.hpp>

namespace zap::builders {

cmake::cmake(
    const zap::env& e,
    const archive_info& ai,
    const zap::strings& args
)
: builder_base(e, ai, args)
{
    cmake_.cmd = zap::find_cmd("cmake");
    build_dir_ = zap::cat_dir(ai.dir, "build");
    stage_dir_ = zap::cat_dir(ai.dir, "stage");
    trace_file_ = zap::cat_file(build_dir_, "zap-trace.json");
}

cmake::~cmake()
{}

void
cmake::configure() const
{
    zap::mkpath(build_dir_);

    auto toolchain_file = zap::cat_file(
        e_["etc"], "toolchains", "build.cmake"
    );

    if (!zap::file_exists(toolchain_file)) {
        zap::mkfilepath(toolchain_file);
        zap::cmake::toolchain_file::write(e_.toolchain(), toolchain_file);
    }

    zap::strings args = {
        zap::cat("-DCMAKE_PREFIX_PATH=", e_["root"]),
        zap::cat("-DCMAKE_INSTALL_PREFIX=", e_["root"]),
        zap::cat("-DCMAKE_TOOLCHAIN_FILE=", toolchain_file),
        "-S", ai_.source_dir,
        "-B", build_dir_,
        "-G", "Ninja",
        "-Wno-dev", // trace mode vomits...
        "--trace-expand",
        zap::cat("--trace-redirect=", trace_file_),
        "--trace-format=json-v1"
    };

    if (!args_.empty()) {
        args.insert(args.end(), args_.begin(), args_.end());
    }

    cmake_.run({ .args = std::move(args) });
}

void
cmake::build() const
{
    cmake_.run({
        .args = {
            "--build", build_dir_,
            "--parallel", std::to_string(std::thread::hardware_concurrency())
        }
    });
}

void
cmake::install(zap::package::manifest& pm) const
{
    cmake_.run({
        .args = { "--install", build_dir_ },
        .env = { { "DESTDIR", stage_dir_ } }
    });

    zap::cmake::trace_parser tp(e_.toolchain());

    tp.parse(ai_.source_dir, trace_file_);

    // Note: env root starts with a '/'
    tp.post_install(zap::cat(stage_dir_, e_["root"]));

    std::cout << "WHOAA" << std::endl;
}

const std::string&
cmake::trace_file() const
{ return trace_file_; }

}
