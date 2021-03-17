#pragma once

#include <zap/builder.hpp>
#include <zap/prog.hpp>

namespace zap::builders {

class cmake : public zap::builder_base
{
public:
    cmake(
        const zap::env& e,
        const zap::archive_info& ai,
        const zap::strings& args = {}
    );

    virtual ~cmake();

    void configure() const final;
    void build() const final;
    void install(zap::package::manifest& pm) const final;

    const std::string& trace_file() const;

private:
    zap::prog cmake_;
    zap::prog make_;
    std::string build_dir_;
    std::string stage_dir_;
    std::string trace_file_;
};

}
