#include <zap/toolchains/msvc.hpp>

namespace zap::toolchains {

msvc::msvc(
    const zap::env_paths& ep,
    zap::toolchain_info&& ti,
    zap::executor& exec
)
: zap::toolchain(ep, std::forward<zap::toolchain_info>(ti), exec)
{
    // TODO
}

msvc::~msvc()
{}

}
