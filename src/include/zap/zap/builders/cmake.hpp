#pragma once

#include <zap/builder.hpp>
#include <zap/prog.hpp>

namespace zap::builders {

class cmake : public zap::builder_base
{
public:
    cmake(const zap::toolchain& tc, const zap::archive_info& ai);
    virtual ~cmake();

    void configure() const final;
    void build() const final;
    void install() const final;

private:
    zap::prog cmake_;
    zap::prog make_;
    std::string build_dir_;
    std::string stage_dir_;
};

}
