#pragma once

#include <memory>

#include <zap/config.hpp>
#include <zap/executor.hpp>
#include <zap/toolchain.hpp>
#include <zap/os_info.hpp>
#include <zap/fetcher.hpp>
#include <zap/archive_info.hpp>

namespace zap {

class env
{
public:
    env(const std::string& root = {});
    virtual ~env();

    const std::string& root() const;

    zap::executor& executor() const;

    const zap::os_info& os_info() const;

    const zap::toolchain& toolchain() const;

    const zap::fetcher& fetcher() const;

    const config& cfg() const;

    archive_info download_archive(const std::string& url) const;

private:
    void make_fetcher();

    std::string root_;
    config cfg_;
    executor_ptr executor_ptr_;
    zap::os_info os_info_;
    toolchain_ptr toolchain_ptr_;
    fetcher_ptr fetcher_ptr_;
};

using env_ptr = std::unique_ptr<env>;

}
