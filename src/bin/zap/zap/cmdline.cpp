#include <iostream>
#include <sstream>
#include <optional>

#include <docopt.h>

#include <zap/cmdline.hpp>
#include <zap/env.hpp>
#include <zap/commands/env.hpp>
#include <zap/commands/remote.hpp>
#include <zap/commands/configure.hpp>
#include <zap/commands/build.hpp>
#include <zap/commands/install.hpp>
#include <zap/commands/analyze.hpp>
#include <zap/log.hpp>

namespace zap {

static const char general_usage[] =
R"(Zap - C++ project tool

usage:
    zap [--help] <command> [<args>...]

Options:
    -h, --help   Prints this

Commands:
    env          Manage environments
    remote       Manage remotes
    install      Install software
    configure    Configures project
    analyze      Shows project targets and interfaces


See 'zap help <command>' for more information on a specific command.
)";

static const char env_usage[] =
R"(usage:
    zap env new <name> <directory>
    zap env delete <name>
    zap env ls [<name>]
    zap env lspkgs [<name>]

Manages environments.
)";

static const char remote_usage[] =
R"(usage:
    zap remote new <id> <type> <url>
    zap remote delete <id>
    zap remote ls

Manages remotes.
)";

static const char install_usage[] =
R"(usage:
    zap install [-e <env>] <url> [--] [<args>...]
    zap install [-e <env>] -d <directory> [--] [<args>...]
    zap install [-e <env>] -f <file>

Options:
    -e <env>        Environment to use
    -d <directory>  Installs software from extracted archive in <directory>
    -f <file>       Installs software from list in <file>

The first form allows you to install a software package by specifying a URL.
All subsequent arguments will be forwarded to the package build system.

The second form will install software listed in the specified file where
each line is in the first form, that is:

URL1 [ARGS...]
URL2 [ARGS...]
...
)";

static const char configure_usage[] =
R"(usage:
    zap configure [-e <env>]

Options:
    -e <env>     Environment to use

Configures a project to build with CMake.
)";

static const char analyze_usage[] =
R"(usage:
    zap analyze [-e <env>] <directory>

Options:
    -e <env>     Environment to use
    <directory>  Project directory to analyze

Configures a project to build with CMake.
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
set_opt(
    const docopt::Options& opts,
    const std::string& key,
    T& v,
    Callable&& cb
)
{
    if (!opts.contains(key)) {
        return;
    }

    if (!opts.at(key)) {
        return;
    }

    v = cb(opts.at(key));
}

void
set_opt(
    const docopt::Options& opts,
    const std::string& key,
    bool& v
)
{ set_opt(opts, key, v, [&](const auto& val) { return val.asBool(); }); }

void
set_opt(
    const docopt::Options& opts,
    const std::string& key,
    std::size_t& v
)
{ set_opt(opts, key, v, [&](const auto& val) { return val.asLong(); }); }

void
set_opt(
    const docopt::Options& opts,
    const std::string& key,
    std::string& v
)
{ set_opt(opts, key, v, [&](const auto& val) { return val.asString(); }); }

void
set_opt(
    const docopt::Options& opts,
    const std::string& key,
    strings& v
)
{ set_opt(opts, key, v, [&](const auto& val) { return val.asStringList(); }); }

void
set_env(
    cmdline& cl,
    const docopt::Options& args,
    const std::string& key
)
{
    env_opts opts;

    if (args.contains(key)) {
        if (args.at(key)) {
            cl.env_name = args.at(key).asString();
            opts.name = cl.env_name;
        }
    }

    cl.ep = new_env(opts);
}

void
parse_env(cmdline& cl, const zap::strings& cmd_args)
{
    auto args = docopt::docopt(env_usage, cmd_args, true);

    zap::commands::env_opts opts;

    if (args["new"].asBool()) {
        opts.cmd = zap::commands::env_cmd::new_env;
        set_opt(args, "<directory>", opts.directory);
    } else if (args["delete"].asBool()) {
        opts.cmd = zap::commands::env_cmd::delete_env;
    } else if (args["ls"].asBool()) {
        opts.cmd = zap::commands::env_cmd::ls_env;
    } else if (args["lspkgs"].asBool()) {
        opts.cmd = zap::commands::env_cmd::ls_pkgs;
    }

    set_opt(args, "<name>", opts.name);

    cl.ep = new_env(env_opts{ .no_init = true });
    cl.cp = new_command<zap::commands::env>(cl.env(), opts);
}

void
parse_remote(cmdline& cl, const zap::strings& cmd_args)
{
    auto args = docopt::docopt(remote_usage, cmd_args, true);

    set_env(cl, args, "-e");

    zap::commands::remote_opts opts;

    if (args["new"].asBool()) {
        opts.cmd = zap::commands::remote_cmd::new_remote;
        set_opt(args, "<type>", opts.type);
        set_opt(args, "<url>", opts.url);
    } else if (args["delete"].asBool()) {
        opts.cmd = zap::commands::remote_cmd::delete_remote;
    } else if (args["ls"].asBool()) {
        opts.cmd = zap::commands::remote_cmd::ls_remote;
    }

    set_opt(args, "<id>", opts.id);

    cl.cp = new_command<zap::commands::remote>(cl.env(), opts);
}

void
parse_install(cmdline& cl, const zap::strings& cmd_args)
{
    auto args = docopt::docopt(install_usage, cmd_args, true);

    set_env(cl, args, "-e");

    zap::commands::install_opts opts;

    set_opt(args, "<url>", opts.url);
    set_opt(args, "<args>", opts.args);
    set_opt(args, "-d", opts.directory);
    set_opt(args, "-f", opts.file);

    cl.cp = new_command<zap::commands::install>(cl.env(), opts);
}

void
parse_configure(cmdline& cl, const zap::strings& cmd_args)
{
    auto args = docopt::docopt(configure_usage, cmd_args, true);

    set_env(cl, args, "-e");

    zap::commands::configure_opts opts;

    cl.cp = new_command<zap::commands::configure>(cl.env(), opts);
}

void
parse_analyze(cmdline& cl, const zap::strings& cmd_args)
{
    auto args = docopt::docopt(analyze_usage, cmd_args, true);

    set_env(cl, args, "-e");

    zap::commands::analyze_opts opts;

    set_opt(args, "<directory>", opts.directory);

    cl.cp = new_command<zap::commands::analyze>(cl.env(), opts);
}

using parse_func = void(*)(cmdline&, const zap::strings&);
using parse_map = std::unordered_map<std::string, parse_func>;

parse_map parsers = {
    { "env", &parse_env },
    { "remote", &parse_remote },
    { "install", &parse_install },
    { "configure", &parse_configure },
    { "analyze", &parse_analyze }
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

    if (args["<env>"]) {
        cl.env_name = args["<env>"].asString();
    }

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

    return cl;
}

} // namespace zap
