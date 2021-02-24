#include <zap/toolchains/clang.hpp>

namespace zap::toolchains {

clang::clang(
    const zap::config& config,
    zap::toolchain_info&& ti,
    zap::executor& exec
)
: gcc(config, std::forward<zap::toolchain_info>(ti), exec)
{
    scanner_.push_args({ "-w" });
}

clang::~clang()
{}

}
