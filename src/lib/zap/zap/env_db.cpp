#include <zap/env_db.hpp>
#include <zap/utils.hpp>
#include <zap/db/dbi.hpp>

namespace zap {

struct env_db_spec
{
    static auto make(const std::string& file)
    {
        using namespace sqlite_orm;

        return make_storage(
            file,
            make_table(
                "pkgs",
                make_column("name", &env_db_pkg::name, primary_key()),
                make_column("version", &env_db_pkg::version)
            ).without_rowid(),
            make_table(
                "pkg_files",
                make_column("pkg", &env_db_pkg_file::pkg, primary_key()),
                make_column("file", &env_db_pkg_file::file)
            ).without_rowid()
        );
    }
};

using dbi = zap::db::dbi<env_db_spec>;

// Note: early declaration of private method so the concrete types can be
// deduced
auto&
env_db::db()
{ return dbi::get_db(db_ptr_); }

auto&
env_db::dbi()
{ return dbi::get(db_ptr_); }

env_db::env_db()
{}

env_db::env_db(const std::string& dir)
{ init(dir); }

env_db::~env_db()
{}

void
env_db::init(const std::string& dir)
{
    auto db_dir = cat_dir(dir, ".zap");
    auto db_file = cat_file(db_dir, "state.db");
    bool initial = !file_exists(db_file);

    if (initial) {
        mkpath(db_dir);
    }

    db_ptr_ = dbi::new_storage(db_file);

    db().open_forever();
    db().sync_schema();
}

env_db_pkgs
env_db::packages()
{
    env_db_pkgs pkgs;

    auto tx_cb = [&](zap::scope& scope) {
        using namespace sqlite_orm;

        pkgs = db().get_all<env_db_pkg>(
            order_by(&env_db_pkg::name)
        );
    };

    dbi().exec_read(tx_cb);

    return pkgs;
}

}
