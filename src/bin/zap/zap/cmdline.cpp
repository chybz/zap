#include <iostream>
#include <sstream>
#include <optional>

#include <docopt.h>

#include <zap/cmdline.hpp>
#include <zap/env.hpp>
#include <zap/commands/configure.hpp>
#include <zap/commands/build.hpp>
#include <zap/commands/install.hpp>
#include <zap/log.hpp>

namespace zap {

static const char general_usage[] =
R"(Zap C++ project tool

usage:
    zap [--help] <command> [<args>...]

Options:
    -h, --help   Prints this

Commands:
    install      Install dependencies
    configure    Configures project


See 'zap help <command>' for more information on a specific command.
)";

static const char install_usage[] =
R"(usage:
    zap install [-e <env>] <url> [<args>...]
    zap install [-e <env>] -f <file>

Options:
    -e <env>     Specifies environment to install to
    -f <file>    Installs dependencies from <file>

The first form allows you to install a software package by specifying a URL.
All subsequent arguments will be forwarded to the package build system.

The second form will install dependencies listed in the specified file where
each line will specify a dependency like in the first form, that is:

URL1 [ARGS...]
URL2 [ARGS...]
...
)";

///////////////////////////////////////////////////////////////////////////////
//
// cmdline
//
///////////////////////////////////////////////////////////////////////////////
const zap::env&
cmdline::env() const
{ return *ep; }

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

template <typename T, typename Callable>
void
set_opt(const docopt::value& val, T& v, Callable&& cb)
{
    if (!val) {
        return;
    }

    v = cb();
}

void
set_opt(const docopt::value& val, bool& v)
{ set_opt(val, v, [&] { return val.asBool(); }); }

void
set_opt(const docopt::value& val, std::size_t& v)
{ set_opt(val, v, [&] { return val.asLong(); }); }

void
set_opt(const docopt::value& val, std::string& v)
{ set_opt(val, v, [&] { return val.asString(); }); }

void
set_opt(const docopt::value& val, strings& v)
{ set_opt(val, v, [&] { return val.asStringList(); }); }

void
parse_install(cmdline& cl, const zap::strings& cmd_args)
{
    auto args = docopt::docopt(install_usage, cmd_args, true);

    zap::commands::install_opts opts;

    set_opt(args.at("<file>"), opts.file);
    set_opt(args.at("<url>"), opts.target);
    set_opt(args.at("<args>"), opts.args);

    cl.cp = new_command<zap::commands::install>(cl.env(), opts);
}

using parse_func = void(*)(cmdline&, const zap::strings&);
using parse_map = std::unordered_map<std::string, parse_func>;

parse_map parsers = {
    { "install", &parse_install }
};

parse_func
get_parser(const std::string& cmd)
{
    die_unless(parsers.contains(cmd), "invalid command: ", cmd);

    return parsers.at(cmd);
}

cmdline
parse(int ac, char** av)
{
    auto args = docopt::docopt(
        general_usage,
        { av + 1, av + ac },
        true, // Help
        "Zap 0.1",
        true // options_first
    );

    cmdline cl;
    std::string cmd = args["<command>"].asString();
    const auto& sub_args = args["<args>"].asStringList();

    if (cmd == "help") {
        die_if(sub_args.empty(), "missing command");

        get_parser(sub_args[0])(cl, { "--help" });
    } else {
        zap::strings cmd_args{ cmd };

        cmd_args.insert(cmd_args.end(), sub_args.begin(), sub_args.end());

        get_parser(cmd)(cl, cmd_args);
    }

    cl.exit = true;
    return cl;
}

} // namespace zap
