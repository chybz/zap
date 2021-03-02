#pragma once

#include <string>
#include <memory>

#include <zap/sys_db_types.hpp>
#include <zap/db/storage_base.hpp>

namespace zap {

class sys_db
{
public:
    sys_db();

    virtual ~sys_db();

    bool has_var(const std::string& name) const;
    const std::string& var(const std::string& name) const;
    void var(const std::string& name, const std::string& val);

    void new_env(const std::string& name, const std::string& root);
    void delete_env(const std::string& name);
    std::string default_env() const;

private:
    void load_info();
    void save_info();

    auto& db();
    auto& dbi();

    zap::db::storage_ptr db_ptr_;
    sys_db_info info_;
};

using sys_db_ptr = std::unique_ptr<sys_db>;

template <typename... Args>
sys_db_ptr
new_sys_db(Args&&... args)
{ return std::make_unique<sys_db>(std::forward<Args>(args)...); }

}
