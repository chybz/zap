#include <zap/commands/env.hpp>
#include <zap/env_db.hpp>
#include <zap/text/table.hpp>
#include <zap/log.hpp>

namespace zap::commands {

env::env(const zap::env& e, const env_opts& opts)
: zap::command(e),
opts_(opts)
{}

env::~env()
{}

void
env::operator()()
{
    switch (opts_.cmd) {
        case env_cmd::new_env:
        new_env();
        break;
        case env_cmd::delete_env:
        delete_env();
        break;
        case env_cmd::ls_env:
        ls_env();
        break;
        case env_cmd::ls_pkgs:
        ls_pkgs();
        break;
    }
}

void
env::new_env()
{
    command::env().sys_db().new_env(opts_.name, opts_.directory);

    zap::log("environment ", opts_.name, " created");
}

void
env::delete_env()
{
    auto e = command::env().sys_db().delete_env(opts_.name);

    zap::log(
        "environment ", opts_.name, " deleted\n",
        "you can remove ", e.root
    );
}

void
env::ls_env()
{
    auto& sdb = command::env().sys_db();

    auto envs = sdb.ls_env(opts_.name);

    if (envs.empty()) {
        return;
    }

    zap::text::table t("name", "root");

    for (auto& e : envs) {
        if (sdb.is_default_env(e.name)) {
            e.name += '*';
        }

        t.add_row(e.name, e.root);
    }

    std::cout << t << std::endl;
}

void
env::ls_pkgs()
{
    auto e = command::env().sys_db().get_env(opts_.name);

    zap::env_db edb(e.root);

    zap::text::table t("name", "version");

    for (auto& p : edb.packages()) {
        t.add_row(p.name, p.version);
    }

    std::cout << t << std::endl;
}

}
