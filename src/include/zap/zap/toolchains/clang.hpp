#pragma once

#include <zap/toolchains/gcc.hpp>

namespace zap::toolchains {

class clang : public gcc
{
public:
    clang(
        const zap::config& config,
        zap::toolchain_info&& ti,
        zap::executor& exec
    );

    virtual ~clang();
};

}
