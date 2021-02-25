#pragma once

#include <vector>
#include <memory>

#include <re2/re2.h>

#include <zap/env.hpp>
#include <zap/types.hpp>

namespace zap::toolchains {

class gcc : public toolchain
{
public:
    gcc(
        const zap::env_paths& ep,
        zap::toolchain_info&& ti,
        zap::executor& exec
    );

    virtual ~gcc();

    void scan_files(
        const zap::strings& inc_dirs,
        const std::string& dir,
        const zap::files& f,
        zap::strings& deps
    ) const override;

    zap::strings local_lib_deps(
        const std::string& file,
        const zap::string_set& accepted
    ) const override;

    zap::strings local_lib_deps(
        const std::string& file,
        const zap::string_map& accepted
    ) const override;

protected:
    virtual void configure_std_header_finder(zap::prog& finder) const;

private:
    void find_std_headers();
    void find_std_headers(zap::files& stdh, const string_views& hdirs) const;

    void extract_deps(
        const re2::RE2& re,
        zap::prog_result& res,
        zap::string_set& deps
    ) const;

    template <typename Associative>
    void local_shared_lib_deps(
        const std::string& file,
        const Associative& accepted,
        zap::strings& deps
    ) const;

    template <typename Associative>
    void local_static_lib_deps(
        const std::string& file,
        const Associative& accepted,
        zap::strings& deps
    ) const;

    re2::RE2 extract_line_re_;
};

}
