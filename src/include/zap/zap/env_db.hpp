#pragma once

#include <string>

#include <zap/env_db_types.hpp>
#include <zap/db/storage_base.hpp>

namespace zap {

class env_db
{
public:
    env_db();
    env_db(const std::string& dir);

    virtual ~env_db();

    void init(const std::string& dir);

private:
    auto& db();
    auto& dbi();

    zap::db::storage_ptr db_ptr_;
};

}
