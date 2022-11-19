#include <zap/commands/install.hpp>
#include <zap/builder.hpp>
#include <zap/utils.hpp>

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
    if (!opts_.url.empty()) {
        install_url(opts_.url);
    } else if (!opts_.directory.empty()) {
        install_directory(opts_.directory);
    }
}

void
install::install_url(const std::string& url)
{
    std::cout << "installing " << url << std::endl;

    auto ai = env().download_archive(url);

    install_archive(ai);
}

void
install::install_directory(const std::string& dir)
{
    std::cout << "installing " << dir << std::endl;

    zap::archive_info ai{
        .dir = zap::empty_temp_dir(env()["tmp"]),
        .source_dir = dir
    };

    install_archive(ai);
}

void
install::install_archive(const archive_info& ai)
{
    zap::builder b(env(), ai, opts_.args);
    zap::package::manifest pm;

    b.configure();
    b.build();
    b.install(pm);
}

}
