#include <zap/command/install.hpp>
#include <zap/builder.hpp>

namespace zap::command {

void
install::operator()(const toolchain& tc)
{
    std::cout << "installing " << target << std::endl;

    auto ai = tc.download_archive(target);

    zap::builder b(tc, ai);

    b.configure();
    b.build();
    b.install();
}

}
