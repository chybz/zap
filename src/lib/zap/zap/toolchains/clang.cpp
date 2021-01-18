#include <zap/toolchains/clang.hpp>

namespace zap::toolchains {

clang::clang(zap::toolchain_info&& ti, zap::executor& exec)
: gcc(std::forward<zap::toolchain_info>(ti), exec)
{
    scanner_.push_args({ "-w" });
}

clang::~clang()
{}

}
