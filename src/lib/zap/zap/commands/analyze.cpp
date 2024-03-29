#include <zap/commands/analyze.hpp>
#include <zap/builders/cmake.hpp>
#include <zap/cmake/trace_parser.hpp>
#include <zap/scope.hpp>
#include <zap/zapfile.hpp>
#include <zap/utils.hpp>

namespace zap::commands {

analyze::analyze(const zap::env& e, const analyze_opts& opts)
: zap::command(e),
opts_(opts)
{}

analyze::~analyze()
{}

void
analyze::operator()()
{
    zapfile zf;

    zf.load();

    return;

    zap::scope s;

    auto tmp = zap::empty_temp_dir("/tmp");

    s.push_rmpath(tmp);

    // TODO: find correct project type instead of CMake default below
    zap::builders::cmake cm(
        env(),
        {
            .dir = tmp,
            .source_dir = opts_.directory
        }
    );

    cm.configure();

    zap::cmake::trace_parser tp(env().toolchain());

    tp.parse(opts_.directory, cm.trace_file());

    std::cout << "analyze done." << std::endl;
}

}
