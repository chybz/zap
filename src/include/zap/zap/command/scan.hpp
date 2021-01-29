#pragma once

#include <zap/toolchain.hpp>
#include <zap/files.hpp>
#include <zap/types.hpp>
#include <zap/project.hpp>
#include <zap/resolver.hpp>
#include <zap/resolvers/apt.hpp>
#include <zap/resolve_info.hpp>

namespace zap::command {

struct scan
{
    bool asan = false;
    bool debug = false;

    void operator()(const zap::toolchain& tc);

private:
    void find_targets();
    void scan_targets();
    void scan_targets(zap::targets& ts);
    void scan_target(zap::target& t);

    void scan_target_files(
        zap::target& t,
        const std::string& dir,
        const zap::files& files,
        zap::string_set& deps
    );

    bool is_project_dep(
        const zap::target& t,
        const std::string& dep,
        std::string& lib
    ) const;

    void resolve_deps(
        const zap::resolvers::apt& apt,
        zap::target& t,
        zap::resolve_info& ri
    );

    void resolve_header_deps(
        const zap::resolvers::apt& apt,
        const zap::string_set& headers,
        zap::string_set& pkgs,
        zap::string_set& libs,
        zap::resolve_info& ri
    );

    void add_project_module(const zap::dep_info& info);

    const zap::toolchain& tc() const;

    const zap::toolchain* tc_ptr_ = nullptr;
    project p_;
    resolver_ptrs rps_;
    zap::string_view_set seen_deps_;
};

}
