#pragma once

#include <zap/toolchains/gcc.hpp>

namespace zap::toolchains {

class clang : public gcc
{
public:
    clang(zap::toolchain_info&& ti, zap::executor& exec);
    virtual ~clang();
};

}
