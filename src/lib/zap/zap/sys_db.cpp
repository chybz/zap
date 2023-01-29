#include <zap/sys_db.hpp>
#include <zap/db/dbi.hpp>
#include <zap/env_db.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/remote.hpp>

namespace zap {

struct sys_db_spec
{
    static auto make(const std::string& file)
    {
        using namespace sqlite_orm;

        return make_storage(
            file,
            make_table(
                "info",
                make_column("name", &sys_db_info_val::name, primary_key()),
                make_column("value", &sys_db_info_val::value)
            ).without_rowid(),
            make_table(
                "envs",
                make_column("name", &sys_db_env::name, primary_key()),
                make_column("root", &sys_db_env::root)
            ).without_rowid(),
            make_table(
                "remotes",
                make_column("id", &sys_db_remote::id, primary_key()),
                make_column("url", &sys_db_remote::url),
                make_column("type", &sys_db_remote::type)
            ).without_rowid()
        );
    }
};

using dbi = zap::db::dbi<sys_db_spec>;

// Note: early declaration of private method so the concrete types can be
// deduced
auto&
sys_db::db()
{ return dbi::get_db(db_ptr_); }

auto&
sys_db::dbi()
{ return dbi::get(db_ptr_); }

sys_db::sys_db()
{
    auto dir = cat_dir(home_directory(), ".config", "zap");
    auto db_file = cat_file(dir, "sys.db");
    bool initial = !file_exists(db_file);

    if (initial) {
        mkpath(dir);
    }

    db_ptr_ = dbi::new_storage(db_file);

    db().open_forever();
    db().sync_schema();

    load_info();

    if (initial) {
        new_remote("GH", "https://github.com", "github");
        new_remote("GL", "https://gitlab.com", "gitlab");
    }

    load_remotes();
}

sys_db::~sys_db()
{}

bool
sys_db::has_var(const std::string& name) const
{ return info_.contains(name); }

const std::string&
sys_db::var(const std::string& name) const
{ return info_[name]; }

void
sys_db::set_var(const std::string& name, const std::string& val)
{
    info_[name] = val;

    db().replace(sys_db_info_val{ name, val });
}

void
sys_db::del_var(const std::string& name)
{ db().remove<sys_db_info_val>(name); }

bool
sys_db::has_default_env() const
{ return has_var("default_env"); }

bool
sys_db::is_default_env(const std::string& name) const
{ return has_default_env() && var("default_env") == name; }

std::string
sys_db::default_env() const
{
    std::string de;

    if (has_default_env()) {
        de = var("default_env");
    }

    return de;
}

///////////////////////////////////////////////////////////////////////////////
//
// Environments
//
///////////////////////////////////////////////////////////////////////////////
void
sys_db::new_env(const std::string& name, const std::string& dir)
{
    auto tx_cb = [&](zap::scope& scope) {
        die_unless(
            dir_is_empty(dir),
            "directory ", dir, " is not empty"
        );

        mkpath(dir);

        scope.if_not_ok_rmpath(dir);

        env_db edb(dir);

        dbi().ensure_not_exists<sys_db_env>(
            "environment", &sys_db_env::name, name
        );

        db().insert(sys_db_env{ name, dir });

        if (!has_var("default_env")) {
            set_var("default_env", name);
        }
    };

    dbi().exec_write(tx_cb);
}

sys_db_env
sys_db::delete_env(const std::string& name)
{
    sys_db_env e;

    auto tx_cb = [&](zap::scope& scope) {
        e = dbi().ensure_exists<sys_db_env>(
            "environment", &sys_db_env::name, name
        );

        db().remove<sys_db_env>(name);

        if (has_var("default_env") && var("default_env") == name) {
            del_var("default_env");
        }
    };

    dbi().exec_write(tx_cb);

    return e;
}

sys_db_env
sys_db::get_env(const std::string& name)
{
    sys_db_env e;

    auto tx_cb = [&](zap::scope& scope) {
        e = dbi().ensure_exists<sys_db_env>(
            "environment", &sys_db_env::name, name
        );
    };

    dbi().exec_read(tx_cb);

    return e;
}

sys_db_envs
sys_db::ls_env(const std::string& name)
{
    sys_db_envs envs;

    auto tx_cb = [&](zap::scope& scope) {
        using namespace sqlite_orm;

        if (name.empty()) {
            envs = db().get_all<sys_db_env>();
        } else {
            envs = db().get_all<sys_db_env>(
                where(c(&sys_db_env::name) == name)
            );

            die_if(envs.empty(), "no such environment: ", name);
        }
    };

    dbi().exec_read(tx_cb);

    return envs;
}

///////////////////////////////////////////////////////////////////////////////
//
// Remotes
//
///////////////////////////////////////////////////////////////////////////////
void
sys_db::new_remote(
    const std::string& id,
    const std::string& url,
    const std::string& type
)
{
    check_remote(type);

    dbi().ensure_not_exists<sys_db_remote>("remote", &sys_db_remote::id, id);

    sys_db_remote r{ id, url, type };

    db().insert(r);

    remotes_.try_emplace(r.id, r);
}

sys_db_remote
sys_db::delete_remote(const std::string& id)
{
    sys_db_remote r;

    auto tx_cb = [&](zap::scope& scope) {
        r = dbi().ensure_exists<sys_db_remote>(
            "remote", &sys_db_remote::id, id
        );

        db().remove<sys_db_remote>(id);
    };

    dbi().exec_write(tx_cb);
}

bool
sys_db::has_remote(const std::string& id) const
{ return remotes_.contains(id); }

const sys_db_remote&
sys_db::remote(const std::string& id) const
{ return remotes_.at(id); }

const sys_db_remotes&
sys_db::remotes() const
{ return remotes_; }

void
sys_db::load_remotes()
{
    auto tx_cb = [&](zap::scope& scope) {
        for (auto& r : db().get_all<sys_db_remote>()) {
            remotes_.try_emplace(r.id, r);
        }
    };

    dbi().exec_read(tx_cb);
}

///////////////////////////////////////////////////////////////////////////////
//
// Info
//
///////////////////////////////////////////////////////////////////////////////
void
sys_db::load_info()
{
    auto tx_cb = [&](zap::scope& scope) {
        for (auto& r : db().get_all<sys_db_info_val>()) {
            info_.vm.try_emplace(std::move(r.name), std::move(r.value));
        }
    };

    dbi().exec_read(tx_cb);
}

void
sys_db::save_info()
{}

}
