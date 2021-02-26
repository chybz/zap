#pragma once

#include <memory>

#include <zap/env_db_types.hpp>

namespace zap {

struct db_storage_base
{};

using db_storage_ptr = std::unique_ptr<db_storage_base>;

class env_db
{
public:
    env_db(const std::string& dir);
    virtual ~env_db();

private:
    auto& db();

    db_storage_ptr db_ptr_;
};

}
