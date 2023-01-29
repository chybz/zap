#include <zap/commands/remote.hpp>
#include <zap/text/table.hpp>
#include <zap/log.hpp>

namespace zap::commands {

remote::remote(const zap::env& e, const remote_opts& opts)
: zap::command(e),
opts_(opts)
{}

remote::~remote()
{}

void
remote::operator()()
{
    switch (opts_.cmd) {
        case remote_cmd::new_remote:
        new_remote();
        break;
        case remote_cmd::delete_remote:
        delete_remote();
        break;
        case remote_cmd::ls_remote:
        ls_remote();
        break;
    }
}

void
remote::new_remote()
{
    command::env().sys_db().new_remote(opts_.id, opts_.url, opts_.type);

    zap::log("remote ", opts_.id, " created");
}

void
remote::delete_remote()
{
    command::env().sys_db().delete_remote(opts_.id);

    zap::log("remote ", opts_.id, " deleted");
}

void
remote::ls_remote()
{
    auto& sdb = command::env().sys_db();

    zap::text::table t("id", "type", "url");

    for (const auto& p : sdb.remotes()) {
        const auto& r = p.second;

        t.add_row(r.id, r.type, r.url);
    }

    std::cout << t << std::endl;
}

}
