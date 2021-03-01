#pragma once

#include <string>
#include <memory>

#include <zap/executor.hpp>
#include <zap/toolchain.hpp>
#include <zap/os_info.hpp>
#include <zap/fetcher.hpp>
#include <zap/archive_info.hpp>
#include <zap/env_paths.hpp>
#include <zap/env_db.hpp>

namespace zap {

struct env_opts
{
    std::string name;
};

class env
{
public:
    env(const env_opts& opts);
    virtual ~env();

    void init(const std::string& name);

    const std::string& root() const;
    const std::string& operator[](const std::string& name) const;

    const string_map& build_env() const;

    zap::executor& executor() const;

    const zap::os_info& os_info() const;

    const zap::toolchain& toolchain() const;

    const zap::fetcher& fetcher() const;

    archive_info download_archive(const std::string& url) const;

private:
    void init();
    void make_fetcher();

    env_opts opts_;
    env_db db_;
    std::string root_;
    env_paths paths_;
    executor_ptr executor_ptr_;
    zap::os_info os_info_;
    toolchain_ptr toolchain_ptr_;
    fetcher_ptr fetcher_ptr_;
    string_map build_env_;
};

using env_ptr = std::unique_ptr<env>;

template <typename... Args>
env_ptr
new_env(Args&&... args)
{ return std::make_unique<env>(std::forward<Args>(args)...); }

}
