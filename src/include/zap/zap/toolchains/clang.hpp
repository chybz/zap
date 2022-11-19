#pragma once

#include <zap/toolchains/gcc.hpp>

namespace zap::toolchains {

class clang : public gcc
{
public:
    clang(
        const zap::env_paths& ep,
        zap::toolchain_info&& ti,
        zap::executor& exec
    );

    virtual ~clang();
};

}
