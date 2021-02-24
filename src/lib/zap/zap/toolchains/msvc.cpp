#include <zap/toolchains/msvc.hpp>

namespace zap::toolchains {

msvc::msvc(
    const zap::config& config,
    zap::toolchain_info&& ti,
    zap::executor& exec
)
: zap::toolchain(config, std::forward<zap::toolchain_info>(ti), exec)
{
    // TODO
}

msvc::~msvc()
{}

}
