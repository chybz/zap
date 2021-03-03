#include <zap/commands/install.hpp>
#include <zap/builder.hpp>
#include <zap/cmake/configs.hpp>
#include <zap/pkg_config/configs.hpp>

namespace zap::commands {

install::install(const zap::env& e, const install_opts& opts)
: zap::command(e),
opts_(opts)
{}

install::~install()
{}

void
install::operator()()
{
    if (!opts_.target.empty()) {
        install_target(opts_.target);
    }
}

void
install::install_target(const std::string& target)
{
    std::cout << "installing " << target << std::endl;

    auto ai = env().download_archive(target);

    zap::builder b(env(), ai);

    b.configure();
    b.build();

    auto stage_dir = b.install();

    // Analyse what's produced
    zap::cmake::configs cmc(env(), stage_dir);
    zap::pkg_config::configs pcc(env(), stage_dir);

    std::cout << "installed" << std::endl;
}

}
