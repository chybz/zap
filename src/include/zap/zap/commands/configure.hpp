#pragma once

#include <zap/command.hpp>
#include <zap/files.hpp>
#include <zap/types.hpp>
#include <zap/project.hpp>
#include <zap/resolver.hpp>
#include <zap/resolve_info.hpp>

namespace zap::commands {

struct configure_opts
{
    bool asan = false;
    bool debug = false;
};

class configure : public zap::command
{
public:
    configure(const zap::env& e, const configure_opts& opts);
    virtual ~configure();

    void operator()() final;

private:
    void find_targets();
    void scan_targets();
    void resolve_targets();

    void scan_targets(zap::targets& ts);
    void scan_target(zap::target& t);

    void scan_target_files(
        zap::target& t,
        const std::string& dir,
        const zap::files& files,
        zap::target_deps& deps
    );

    bool is_project_dep(
        const zap::target& t,
        const std::string& dep,
        std::string& lib
    ) const;

    void resolve_targets(
        const zap::resolver& res,
        zap::targets& ts,
        zap::resolve_info& ri
    );

    void resolve_deps(
        const zap::resolver& res,
        zap::target& t,
        zap::resolve_info& ri
    );

    void resolve_header_deps(
        const zap::resolver& res,
        zap::target_deps& deps,
        zap::string_set& pkgs,
        zap::resolve_info& ri
    );

    void add_project_module(const zap::dep_info& info);

    void project_info(
        std::ostream& os,
        const zap::resolver& res,
        const zap::resolve_info& ri
    ) const;

    void project_pkg_info(
        std::ostream& os,
        const std::string& label,
        const zap::string_set& pkgs
    ) const;

    void project_targets_info(
        std::ostream& os,
        const std::string& label,
        const zap::targets& ts
    ) const;

    const zap::env& e() const;

    configure_opts opts_;
    project p_;
    resolver_ptrs rps_;
};

}
