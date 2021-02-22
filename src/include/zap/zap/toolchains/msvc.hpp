#pragma once

#include <zap/env.hpp>

namespace zap::toolchains {

class msvc : public toolchain
{
public:
    msvc(zap::toolchain_info&& ti, zap::executor& exec);
    virtual ~msvc();
};

}
