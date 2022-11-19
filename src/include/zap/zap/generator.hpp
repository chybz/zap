#pragma once

#include <zap/env.hpp>
#include <zap/project.hpp>

namespace zap {

class generator
{
public:
    generator(
        const zap::env& e,
        const project& p
    );
    virtual ~generator();

    const zap::env& env() const;
    const project& p() const;

    virtual void generate() = 0;

private:
    const zap::env& e_;
    const project& p_;
};

}
