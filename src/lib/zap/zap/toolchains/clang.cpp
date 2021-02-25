#include <zap/toolchains/clang.hpp>

namespace zap::toolchains {

clang::clang(
    const zap::env_paths& ep,
    zap::toolchain_info&& ti,
    zap::executor& exec
)
: gcc(ep, std::forward<zap::toolchain_info>(ti), exec)
{
    scanner_.push_args({ "-w" });
}

clang::~clang()
{}

}
