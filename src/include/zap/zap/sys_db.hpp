#pragma once

#include <string>

#include <zap/sys_db_types.hpp>
#include <zap/db/storage_base.hpp>

namespace zap {

class sys_db
{
public:
    sys_db();

    virtual ~sys_db();

    void new_env(const std::string& name, const std::string& root);
    void delete_env(const std::string& name);
    std::string default_env() const;

private:
    auto& db();
    auto& dbi();

    zap::db::storage_ptr db_ptr_;
};

}
