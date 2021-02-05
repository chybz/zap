#pragma once

#include <fstream>

#include <zap/generator.hpp>

namespace zap::generators {

class cmake : public zap::generator
{
public:
    cmake(const toolchain& tc, const zap::project& p);
    virtual ~cmake();

    void generate() final;

private:
    std::ofstream ofs_;
};

}
