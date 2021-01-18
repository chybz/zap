#pragma once

#include <zap/builder.hpp>
#include <zap/prog.hpp>

namespace zap::builders {

class autotools : public zap::builder_base
{
public:
    autotools(const zap::toolchain& tc, const zap::archive_info& ai);
    virtual ~autotools();

    void configure() const final;
    void build() const final;
    void install() const final;

private:
    zap::prog make_;
    std::string build_dir_;
    std::string stage_dir_;
};

}
