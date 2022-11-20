#pragma once

#include <zap/command.hpp>
#include <zap/files.hpp>
#include <zap/types.hpp>
#include <zap/project.hpp>

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

    const zap::env& e() const;

    configure_opts opts_;
    project p_;
};

}
