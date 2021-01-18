#pragma once

#include <string>
#include <memory>

#include <zap/config.hpp>
#include <zap/prog.hpp>
#include <zap/toolchain_info.hpp>
#include <zap/files.hpp>
#include <zap/types.hpp>
#include <zap/fetcher.hpp>
#include <zap/archive_info.hpp>
#include <zap/os_info.hpp>
#include <zap/executor.hpp>
#include <zap/frameworks.hpp>

namespace zap {

class toolchain
{
public:
    toolchain(toolchain_info&& ti, executor& exec);
    virtual ~toolchain();

    executor& exec() const;

    const std::string& target_arch() const;

    strings make_arch_dirs(
        const std::string& root,
        const std::string& pre,
        const std::string& post = {}
    ) const;

    const prog& cxx() const;
    const prog& cc() const;
    const prog& nm() const;
    const prog& ldd() const;
    const prog& scanner() const;
    const zap::fetcher& fetcher() const;
    const zap::frameworks& frameworks() const;

    const config& cfg() const;

    const files& std_headers() const;
    bool is_std_header(const std::string& name) const;

    strings scan_files(
        const strings& inc_dirs,
        const std::string& dir,
        const files& f
    ) const;

    virtual void scan_files(
        const strings& inc_dirs,
        const std::string& dir,
        const files& f,
        strings& deps
    ) const;

    virtual strings local_lib_deps(
        const std::string& file,
        const string_set& accepted
    ) const;

    virtual strings local_lib_deps(
        const std::string& file,
        const string_map& accepted
    ) const;

    archive_info download_archive(const std::string& url) const;

    bool is_unknown() const;
    bool is_gcc() const;
    bool is_native_clang() const;
    bool is_apple_clang() const;
    bool is_clang() const;
    bool is_msvc() const;

    const std::string& name() const;

    const zap::os_info& os_info() const;

protected:
    prog& cxx();
    prog& cc();
    prog& nm();
    prog& ldd();
    prog& scanner();

    toolchain_info info_;
    std::string target_arch_;
    prog scanner_;
    files std_headers_;

private:
    void find_libc_headers();
    void make_fetcher();

    fetcher_ptr fetcher_ptr_;
    zap::os_info os_info_;
    executor& executor_;
    zap::frameworks frameworks_;
};

const toolchain&
get_toolchain();

}
