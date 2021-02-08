#include <iostream>
#include <sstream>

#include <lyra/lyra.hpp>

#include <zap/cmdline.hpp>
#include <zap/toolchain.hpp>

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// cmdline
//
///////////////////////////////////////////////////////////////////////////////
void
cmdline::run()
{ std::visit([&](auto&& c) { c(zap::get_toolchain()); }, c); }

///////////////////////////////////////////////////////////////////////////////
//
// helpers
//
///////////////////////////////////////////////////////////////////////////////
template <typename Command>
void
assign_command(
    const lyra::group& g,
    cmdline& cl,
    Command&& cmd,
    bool show_help
)
{
    if (show_help) {
        std::cout << g;
    } else {
        cl.c = std::move(cmd);
    }
}

struct configure_cmd
{
    command::configure c;
    bool show_help = false;

    configure_cmd(lyra::cli& cli, cmdline& cl)
    {
        cli.add_argument(
            lyra::command(
                "configure",
                [&](const lyra::group& g) {
                    assign_command(g, cl, c, show_help);
                }
            )
            .help("configures local project")
            .add_argument(lyra::help(show_help))
        );
    }
};

struct install_cmd
{
    command::install c;
    bool show_help = false;

    install_cmd(lyra::cli& cli, cmdline& cl)
    {
        cli.add_argument(
            lyra::command(
                "install",
                [&](const lyra::group& g) {
                    assign_command(g, cl, c, show_help);
                }
            )
            .help("install library in local environment")
            .add_argument(lyra::help(show_help))
            .add_argument(
                lyra::arg(c.target, "library")
                .help("URL to install from")
                .required()
            )
        );
    }
};

cmdline
parse(int ac, char** av)
{
    auto cli = lyra::cli();
    cmdline cl;
    std::string cmd;
    bool show_help = false;

    cli.add_argument(lyra::help(show_help));

    configure_cmd cc(cli, cl);
    install_cmd ic(cli, cl);

    auto result = cli.parse({ ac, av });

    if (show_help) {
        std::cout << cli;
    } else if (!result) {
        cl.exit = true;
        std::cerr << result.errorMessage() << std::endl;
    }

    return cl;
}

} // namespace zap
