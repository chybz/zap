#include <zap/sys_db.hpp>
#include <zap/db/dbi.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

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
                make_column("default_env", &sys_db_info::default_env)
            ).without_rowid(),
            make_table(
                "envs",
                make_column("name", &sys_db_env::name, primary_key()),
                make_column("root", &sys_db_env::root)
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
}

sys_db::~sys_db()
{}

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

        dbi().ensure_not_exists<sys_db_env>("an environment named", name);

        db().insert(sys_db_env{ name, dir });

        dbi().with_record<sys_db_info>(
            [&](auto& info) {
                bool modified = false;

                if (info.default_env.empty()) {
                    info.default_env = name;
                    modified = true;
                }

                return modified;
            }
        );
    };

    dbi().exec_write(tx_cb);
}

void
sys_db::delete_env(const std::string& name)
{}

std::string
sys_db::default_env() const
{}

}
