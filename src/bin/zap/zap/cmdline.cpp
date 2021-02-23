#include <iostream>
#include <sstream>
#include <optional>

#include <structopt/app.hpp>

#include <zap/cmdline.hpp>
#include <zap/env.hpp>

namespace zap {

struct options
{
    std::optional<bool> help = false;
    std::optional<bool> verbose = false;

    struct configure_opts
    : structopt::sub_command
    {
        std::optional<bool> pouet;
    };

    // struct build_opts
    // : zap::commands::build_opts, structopt::sub_command
    // {};

    // struct install_opts
    // : zap::commands::install_opts, structopt::sub_command
    // {};

    configure_opts configure;
    // build_opts build;
    // install_opts install;
};

STRUCTOPT(options::configure_opts, pouet);
STRUCTOPT(options, help, verbose, configure);

// STRUCTOPT(options::configure, asan, debug);
// STRUCTOPT(options::build, cpus);
// STRUCTOPT(options::install, target);
// STRUCTOPT(options, help, verbose, configure, build, install);

///////////////////////////////////////////////////////////////////////////////
//
// cmdline
//
///////////////////////////////////////////////////////////////////////////////
void
cmdline::run()
{}

///////////////////////////////////////////////////////////////////////////////
//
// helpers
//
///////////////////////////////////////////////////////////////////////////////
cmdline
parse(const zap::env& e, int ac, char** av)
{
    try {
        // Line of code that does all the work:
        auto opts = structopt::app("zap").parse<options>(argc, argv);
    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
    }


    cmdline cl;

    return cl;
}

} // namespace zap
