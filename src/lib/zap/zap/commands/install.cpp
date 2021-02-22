#include <zap/commands/install.hpp>
#include <zap/builder.hpp>

namespace zap::commands {

install::install(const zap::env& e)
: zap::command(e)
{}

install::~install()
{}

void
install::operator()()
{
    //TOFIX
    // std::cout << "installing " << target << std::endl;

    // auto ai = e.download_archive(target);

    // zap::builder b(env(), ai);

    // b.configure();
    // b.build();
    // b.install();
}

}
