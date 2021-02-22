#pragma once

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
    env();
    virtual ~env();

    zap::executor& executor() const;

    const zap::os_info& os_info() const;

    const zap::toolchain& toolchain() const;

    const zap::fetcher& fetcher() const;

    const config& cfg() const;

    archive_info download_archive(const std::string& url) const;

private:
    void make_fetcher();

    config cfg_;
    executor_ptr executor_ptr_;
    zap::os_info os_info_;
    toolchain_ptr toolchain_ptr_;
    fetcher_ptr fetcher_ptr_;
};

}
