#pragma once

#include <zap/builder.hpp>
#include <zap/prog.hpp>

namespace zap::builders {

class autotools : public zap::builder_base
{
public:
    autotools(const zap::env& e, const zap::archive_info& ai);
    virtual ~autotools();

    void configure() const final;
    void build() const final;
    const std::string& install() const final;

private:
    zap::prog make_;
    std::string build_dir_;
    std::string stage_dir_;
};

}
