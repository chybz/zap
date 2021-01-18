#pragma once

#include <zap/toolchain.hpp>

namespace zap::toolchains {

class msvc : public toolchain
{
public:
    msvc(zap::toolchain_info&& ti, zap::executor& exec);
    virtual ~msvc();
};

}
