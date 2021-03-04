#include <zap/commands/install.hpp>
#include <zap/builder.hpp>

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
    zap::package::manifest pm;

    b.configure();
    b.build();
    b.install(pm);
}

}
