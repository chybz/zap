#pragma once

#include <string>
#include <memory>

#include <zap/prog.hpp>
#include <zap/toolchain_info.hpp>
#include <zap/files.hpp>
#include <zap/types.hpp>
#include <zap/executor.hpp>
#include <zap/env_paths.hpp>
#include <zap/scope.hpp>

namespace zap {

class toolchain
{
public:
    toolchain(const zap::env_paths& ep, toolchain_info&& ti, zap::executor& e);
    virtual ~toolchain();

    bool link_shared() const;
    bool link_static() const;

    const std::string& target_arch() const;

    void make_arch_dirs(
        strings& dirs,
        const std::string& root,
        const std::string& pre,
        const std::string& post = {}
    ) const;

    strings make_arch_dirs(
        const std::string& root,
        const std::string& pre,
        const std::string& post = {}
    ) const;

    strings make_arch_conf_dirs(
        const std::string& root,
        const std::string& post
    ) const;

    const prog& cc() const;
    const prog& cxx() const;
    const prog& nm() const;
    const prog& ldd() const;
    const prog& scanner() const;

    bool has_compiler_launcher() const;
    const std::string& compiler_launcher_cmd() const;

    const std::string& cc_cmd() const;
    const std::string& cxx_cmd() const;

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

    bool is_unknown() const;
    bool is_gcc() const;
    bool is_native_clang() const;
    bool is_apple_clang() const;
    bool is_clang() const;
    bool is_msvc() const;

    const std::string& name() const;

protected:
    const std::string& empty_dir() const;
    const std::string& empty_file() const;

    const zap::env_paths& ep() const;
    zap::executor& executor() const;

    void set_target_arch(const std::string& arch);

    prog& cxx();
    prog& cc();
    prog& nm();
    prog& ldd();
    prog& scanner();

    const zap::env_paths& ep_;
    toolchain_info info_;
    std::string target_arch_;
    prog scanner_;
    files std_headers_;
    std::string empty_dir_;
    std::string empty_file_;
    scope scope_;

private:
    void find_libc_headers();

    zap::executor& executor_;
};

using toolchain_ptr = std::unique_ptr<toolchain>;

toolchain_ptr
make_toolchain(const zap::env_paths& ep, zap::executor& e);

}
