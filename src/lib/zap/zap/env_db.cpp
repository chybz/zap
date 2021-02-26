#include <sqlite_orm/sqlite_orm.h>

#include <zap/env_db.hpp>
#include <zap/utils.hpp>

namespace zap {

///////////////////////////////////////////////////////////////////////////////
//
// A bit kludgy but avoids exposing sqlite_orm in interface
//
///////////////////////////////////////////////////////////////////////////////
auto
make_db_storage(const std::string& dbfile)
{
    using namespace sqlite_orm;

    return make_storage(
        dbfile,
        make_table(
            "pkgs",
            make_column("name", &pkg::name, primary_key()),
            make_column("version", &pkg::version)
        ).without_rowid(),
        make_table(
            "pkg_files",
            make_column("pkg", &pkg_file::pkg, primary_key()),
            make_column("file", &pkg_file::file)
        ).without_rowid()
    );
}

using db_storage_type = decltype(make_db_storage(std::string{}));

struct db_storage : db_storage_base
{
    db_storage(const std::string& file)
    : db(make_db_storage(file))
    {}

    ~db_storage()
    {}

    db_storage_type db;
};

// Note: early declaration of private method so the concrete types can be
// deduced
auto&
env_db::db()
{ return static_cast<db_storage&>(*db_ptr_).db; }

env_db::env_db(const std::string& dir)
{
    auto db_dir = cat_dir(dir, ".zap");
    auto db_file = cat_file(db_dir, "state.db");
    bool initial = !file_exists(db_file);

    if (initial) {
        mkpath(db_dir);
    }

    db_ptr_ = std::make_unique<db_storage>(db_file);

    db().open_forever();
    db().sync_schema();
}

env_db::~env_db()
{}

}
