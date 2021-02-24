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
    std::cout << "installing " << target << std::endl;

    auto ai = env().download_archive(target);

    zap::builder b(env(), ai);

    b.configure();
    b.build();
    b.install();
}

}
