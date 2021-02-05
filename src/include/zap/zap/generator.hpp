#pragma once

#include <zap/toolchain.hpp>
#include <zap/project.hpp>

namespace zap {

class generator
{
public:
    generator(
        const toolchain& tc,
        const project& p
    );
    virtual ~generator();

    const toolchain& tc() const;
    const project& p() const;

    virtual void generate() = 0;

private:
    const toolchain& tc_;
    const project& p_;
};

}
