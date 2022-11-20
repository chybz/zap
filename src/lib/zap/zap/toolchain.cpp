#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <memory>

#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/toolchain.hpp>
#include <zap/toolchains/gcc.hpp>
#include <zap/toolchains/clang.hpp>
#include <zap/toolchains/msvc.hpp>

namespace zap {

using toolchain_ptr = std::unique_ptr<toolchain>;

struct default_cmds
{
    std::string cxx;
    std::string cc;
    std::string nm;
    std::string ldd;
    std::string fetcher;
};

const default_cmds&
get_default_cmds()
{
    static default_cmds dcs = {
#if defined(_WIN32)
        "cl", "cl", "dumpbin", "depends", "curl"
#elif defined(__APPLE__)
        "c++", "cc", "nm", "otool", "curl"
#elif defined(__unix__)
        "c++", "cc", "nm", "ldd", "curl"
#endif
    };

    return dcs;
}

toolchain::toolchain(
    const zap::env_paths& ep,
    toolchain_info&& ti,
    zap::executor& e
)
: ep_(ep),
info_(std::move(ti)),
executor_(e)
{
    empty_dir_ = zap::empty_temp_dir(ep["tmp"]);

    zap::mkpath(empty_dir_);
    scope_.push_rmpath(empty_dir_);

    empty_file_ = zap::cat_file(empty_dir_, "empty.cpp");

    zap::touch_file(empty_file_);
}

toolchain::~toolchain()
{}

const std::string&
toolchain::target_arch() const
{ return target_arch_; }

void
toolchain::make_arch_dirs(
    strings& dirs,
    const std::string& root,
    const std::string& pre,
    const std::string& post
) const
{
    if (post.empty()) {
        dirs.emplace_back(cat_dir(root, pre));
        dirs.emplace_back(cat_dir(root, pre, target_arch_));
    } else {
        dirs.emplace_back(cat_dir(root, pre, post));
        dirs.emplace_back(cat_dir(root, pre, target_arch_, post));
    }
}

strings
toolchain::make_arch_dirs(
    const std::string& root,
    const std::string& pre,
    const std::string& post
) const
{
    strings dirs;

    make_arch_dirs(dirs, root, pre, post);

    return dirs;
}

strings
toolchain::make_arch_conf_dirs(
    const std::string& root,
    const std::string& post
) const
{
    strings dirs;

    make_arch_dirs(dirs, root, "lib", post);
    make_arch_dirs(dirs, root, "share", post);

    return dirs;
}

const std::string&
toolchain::empty_dir() const
{ return empty_dir_; }

const std::string&
toolchain::empty_file() const
{ return empty_file_; }

const zap::env_paths&
toolchain::ep() const
{ return ep_; }

zap::executor&
toolchain::executor() const
{ return executor_; }

void
toolchain::set_target_arch(const std::string& arch)
{
    target_arch_ = arch;

    find_libc_headers();
}

const prog&
toolchain::cxx() const
{ return info_.cxx; }

const prog&
toolchain::cc() const
{ return info_.cc; }

const prog&
toolchain::nm() const
{ return info_.nm; }

const prog&
toolchain::ldd() const
{ return info_.ldd; }

const prog&
toolchain::scanner() const
{ return scanner_; }

bool
toolchain::has_compiler_launcher() const
{ return !compiler_launcher_cmd().empty(); }

const std::string&
toolchain::compiler_launcher_cmd() const
{ return info_.compiler_launcher.cmd; }

const std::string&
toolchain::cc_cmd() const
{ return cc().cmd; }

const std::string&
toolchain::cxx_cmd() const
{ return cxx().cmd; }

const files&
toolchain::std_headers() const
{ return std_headers_; }

bool
toolchain::is_std_header(const std::string& name) const
{ return std_headers_.contains(name); }

strings
toolchain::scan_files(
    const strings& inc_dirs,
    const std::string& dir,
    const files& f
) const
{
    strings deps;

    scan_files(inc_dirs, dir, f, deps);

    return deps;
}

void
toolchain::scan_files(
    const strings& inc_dirs,
    const std::string& dir,
    const files& f,
    strings& deps
) const
{}

strings
toolchain::local_lib_deps(
    const std::string& file,
    const string_set& accepted
) const
{ return {}; }

strings
toolchain::local_lib_deps(
    const std::string& file,
    const string_map& accepted
) const
{ return {}; }

bool
toolchain::is_unknown() const
{ return info_.type == toolchain_type::unknown; }

bool
toolchain::is_gcc() const
{ return info_.type == toolchain_type::gcc; }

bool
toolchain::is_native_clang() const
{ return info_.type == toolchain_type::clang; }

bool
toolchain::is_apple_clang() const
{ return info_.type == toolchain_type::apple_clang; }

bool
toolchain::is_clang() const
{ return is_native_clang() || is_apple_clang(); }

bool
toolchain::is_msvc() const
{ return info_.type == toolchain_type::msvc; }

const std::string&
toolchain::name() const
{ return toolchain_name(info_.type); }

prog&
toolchain::cxx()
{ return info_.cxx; }

prog&
toolchain::cc()
{ return info_.cc; }

prog&
toolchain::nm()
{ return info_.nm; }

prog&
toolchain::ldd()
{ return info_.ldd; }

prog&
toolchain::scanner()
{ return scanner_; }

void
toolchain::find_libc_headers()
{
    if (os_info().is_debian()) {
        auto res = run("dpkg", { .args = { "-L", "libc6-dev" } });
        auto lines = res.get_lines();

        std::string hfilter{
            "/usr/include/(?:" + target_arch() + "/)?(.*\\.h)"
        };

        for (auto& hdr : map_lines(hfilter, lines)) {
            std_headers_.insert(std::string{hdr});
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
///////////////////////////////////////////////////////////////////////////////
template <typename ToolChain, typename... Args>
toolchain_ptr
new_toolchain(Args&&... args)
{ return std::make_unique<ToolChain>(std::forward<Args>(args)...); }

toolchain_ptr
make_toolchain(const zap::env_paths& ep, zap::executor& e)
{
    toolchain_info ti;

    auto dcs = get_default_cmds();

    ti.cxx.cmd = find_cmd(dcs.cxx, "CXX");
    ti.cc.cmd = find_cmd(dcs.cc, "CC");
    ti.nm.cmd = find_cmd(dcs.nm, "NM");
    ti.ldd.cmd = find_cmd(dcs.ldd, "LDD");

    detect_toolchain(ti);

    die_if(
        ti.type == toolchain_type::unknown,
        "unsupported compiler: ", ti.cxx.cmd
    );

    toolchain_ptr tcp;

    switch (ti.type) {
        case toolchain_type::gcc:
        tcp = new_toolchain<zap::toolchains::gcc>(ep, std::move(ti), e);
        break;
        case toolchain_type::clang:
        case toolchain_type::apple_clang:
        tcp = new_toolchain<zap::toolchains::clang>(ep, std::move(ti), e);
        break;
        case toolchain_type::msvc:
        tcp = new_toolchain<zap::toolchains::msvc>(ep, std::move(ti), e);
        default:
        break;
    }

    return tcp;
}

}
