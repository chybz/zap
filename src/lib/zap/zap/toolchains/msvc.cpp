#include <zap/toolchains/msvc.hpp>

namespace zap::toolchains {

msvc::msvc(zap::toolchain_info&& ti, zap::executor& exec)
: zap::toolchain(std::forward<zap::toolchain_info>(ti), exec)
{
    // TODO
}

msvc::~msvc()
{}

}
