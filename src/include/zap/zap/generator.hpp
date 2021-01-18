#pragma once

#include <zap/project.hpp>

namespace zap {

class generator
{
public:
    generator(const project& p);
    virtual ~generator();

private:
    const project& p_;
};

}
