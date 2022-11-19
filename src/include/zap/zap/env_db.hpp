#pragma once

#include <string>
#include <memory>

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

    env_db_pkgs packages();

    bool has_archive(const std::string& url, env_db_archive& ar);
    void add_archive(const env_db_archive& ar);

private:
    auto& db();
    auto& dbi();

    zap::db::storage_ptr db_ptr_;
};

using env_db_ptr = std::unique_ptr<env_db>;

template <typename... Args>
env_db_ptr
new_env_db(Args&&... args)
{ return std::make_unique<env_db>(std::forward<Args>(args)...); }

}
