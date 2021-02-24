#include <iostream>
#include <sstream>
#include <optional>

#include <structopt/app.hpp>

#include <zap/cmdline.hpp>
#include <zap/env.hpp>
#include <zap/commands/configure.hpp>
#include <zap/commands/build.hpp>
#include <zap/commands/install.hpp>

// Note: until fixed by upstream, this must be in global namespace
struct zap_options
{
    std::optional<bool> help = false;
    std::optional<bool> verbose = false;

    struct configure_opts
    : zap::commands::configure_opts, structopt::sub_command
    {};

    struct build_opts
    : zap::commands::build_opts, structopt::sub_command
    {};

    struct install_opts
    : zap::commands::install_opts, structopt::sub_command
    {};

    configure_opts configure;
    build_opts build;
    install_opts install;
};

STRUCTOPT(zap_options::configure_opts, asan, debug);
STRUCTOPT(zap_options::build_opts, cpus);
STRUCTOPT(zap_options::install_opts, target);
STRUCTOPT(zap_options, help, verbose, configure, build, install);

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// cmdline
//
///////////////////////////////////////////////////////////////////////////////
void
cmdline::run()
{ (*cp)(); }

///////////////////////////////////////////////////////////////////////////////
//
// helpers
//
///////////////////////////////////////////////////////////////////////////////
void
print_error(const std::string& what, const std::string& help)
{
    std::cout
        << what << "\n"
        << help
        << std::flush
        ;
}

cmdline
parse(const zap::env& e, int ac, char** av)
{
    cmdline cl;

    try {
        // Line of code that does all the work:
        structopt::app app("zap");

        auto opts = app.parse<zap_options>(ac, av);

        if (opts.configure.has_value()) {
            cl.cp = new_command<zap::commands::configure>(e, opts.configure);
        } else if (opts.build.has_value()) {
            cl.cp = new_command<zap::commands::build>(e, opts.build);
        } else if (opts.install.has_value()) {
            cl.cp = new_command<zap::commands::install>(e, opts.install);
        } else {
            cl.exit = true;

            print_error("Error: missing subcommand", app.help());
        }
    } catch (structopt::exception& ex) {
        cl.exit = true;

        print_error(ex.what(), ex.help());
    }

    return cl;
}

} // namespace zap
