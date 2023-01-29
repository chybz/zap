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

    bool has_default_env() const;
    bool is_default_env(const std::string& name) const;
    std::string default_env() const;

    void new_env(const std::string& name, const std::string& root);
    sys_db_env delete_env(const std::string& name);
    sys_db_env get_env(const std::string& name);
    sys_db_envs ls_env(const std::string& name = {});

    void new_remote(
        const std::string& id,
        const std::string& url,
        const std::string& type
    );
    sys_db_remote delete_remote(const std::string& id);

    bool has_remote(const std::string& id) const;
    const sys_db_remote& remote(const std::string& id) const;
    const sys_db_remotes& remotes() const;

private:
    // Private use for now, it inserts into db
    void set_var(const std::string& name, const std::string& val);
    void del_var(const std::string& name);

    void load_remotes();

    void load_info();
    void save_info();

    auto& db();
    auto& dbi();

    zap::db::storage_ptr db_ptr_;
    sys_db_info info_;
    sys_db_remotes remotes_;
};

using sys_db_ptr = std::unique_ptr<sys_db>;

template <typename... Args>
sys_db_ptr
new_sys_db(Args&&... args)
{ return std::make_unique<sys_db>(std::forward<Args>(args)...); }

}
