#pragma once

#include <zap/generator.hpp>

namespace zap::generators {

class cmake : public zap::generator
{
public:
    cmake(const zap::project& p);
    virtual ~cmake();

private:
};

}
