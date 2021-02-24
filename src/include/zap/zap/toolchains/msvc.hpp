#pragma once

#include <zap/env.hpp>

namespace zap::toolchains {

class msvc : public toolchain
{
public:
    msvc(
        const zap::config& config,
        zap::toolchain_info&& ti,
        zap::executor& exec
    );

    virtual ~msvc();
};

}
