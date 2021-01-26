#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <memory>

#include <taskflow/taskflow.hpp>

#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/toolchain.hpp>
#include <zap/toolchains/gcc.hpp>
#include <zap/toolchains/clang.hpp>
#include <zap/toolchains/msvc.hpp>
#include <zap/fetchers/curl.hpp>
#include <zap/archiver.hpp>
#include <zap/scope.hpp>

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

toolchain::toolchain(toolchain_info&& ti, executor& exec)
: info_(std::move(ti)),
executor_(exec)
{}

toolchain::~toolchain()
{}

executor&
toolchain::exec() const
{ return executor_; }

const std::string&
toolchain::target_arch() const
{ return target_arch_; }

strings
toolchain::make_arch_dirs(
    const std::string& root,
    const std::string& pre,
    const std::string& post
) const
{
    strings dirs;

    if (post.empty()) {
        dirs.emplace_back(cat_dir(root, pre));
        dirs.emplace_back(cat_dir(root, pre, target_arch_));
    } else {
        dirs.emplace_back(cat_dir(root, pre, post));
        dirs.emplace_back(cat_dir(root, pre, target_arch_, post));
    }

    return dirs;
}

void
toolchain::set_target_arch(const std::string& arch)
{
    target_arch_ = arch;

    find_libc_headers();
    make_fetcher();
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

const zap::fetcher&
toolchain::fetcher() const
{ return *fetcher_ptr_; }

const zap::frameworks&
toolchain::frameworks() const
{ return frameworks_; }

const config&
toolchain::cfg() const
{ return info_.cfg; }

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

archive_info
toolchain::download_archive(const std::string& url) const
{
    scope s;
    archive_info ai{url};

    mkpath(cfg().archives_dir);
    auto tdir = empty_temp_dir(cfg().archives_dir);

    s.push_rmpath(tdir);

    fetcher().download(url, tdir);

    auto [ dlok, file ] = unique_file(tdir);

    die_unless(
        dlok,
        "unable to find dowloaded file in: ", tdir
    );

    ai.file = cat_file(cfg().archives_dir, file);

    rename(cat_file(tdir, file), ai.file);

    archiver ar(cfg(), ai.file);

    ar.verify();
    ar.extract(tdir);

    auto [ exok, dir ] = unique_dir(tdir);

    die_unless(
        exok,
        "archive extracts into multiple directories: ", url
    );

    auto pos = dir.rfind('-');

    die_if(
        pos == std::string::npos,
        "unable to extract name and version from: ", dir
    );

    ai.dir = cat_dir(cfg().work_dir, dir);

    die_if(
        directory_exists(ai.dir),
        "source directory already exists: ", ai.dir
    );

    mkpath(ai.dir);

    ai.name = dir.substr(0, pos);
    ai.version = dir.substr(pos + 1);
    ai.source_dir = cat_dir(ai.dir, "src");

    rename(cat_dir(tdir, dir), ai.source_dir);

    // TODO: register and cache

    return ai;
}

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

const zap::os_info&
toolchain::os_info() const
{ return os_info_; }

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
        auto res = run("dpkg", { "-L", "libc6-dev" });
        auto lines = res.get_lines();

        std::string hfilter{
            "/usr/include/(?:" + target_arch() + "/)?(.*\\.h)"
        };

        for (auto& hdr : map_lines(hfilter, lines)) {
            std_headers_.push_back(std::string{hdr});
        }
    }
}

void
toolchain::make_fetcher()
{
    auto ftype = get_default_cmds().fetcher;

    if (ftype == "curl") {
        fetcher_ptr_ = new_fetcher<zap::fetchers::curl>(cfg());
    } else {
        die("unsupported fetcher: ", ftype);
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

void
make_toolchain(toolchain_ptr& tcp, tf::Executor& exec)
{
    toolchain_info ti;
    auto& cfg = ti.cfg;

    namespace fs = std::filesystem;

    auto curp = fs::current_path();
    auto buildp = curp / "build";

    cfg.project_dir = curp.string();
    cfg.build_dir = buildp.string();
    cfg.archives_dir = (buildp / "archives").string();
    cfg.work_dir = (buildp / "work").string();
    cfg.local_prefix = "/local";
    cfg.empty_dir = cat_dir(cfg.home, "scan", "empty");
    cfg.empty_source_file = cat_file(cfg.home, "scan", "empty.cpp");

    mkpath(cfg.empty_dir);
    touch_file(cfg.empty_source_file);

    cfg.meta_file = (curp / "cppmeta.yml").string();
    cfg.load_meta();

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

    switch (ti.type) {
        case toolchain_type::gcc:
        tcp = new_toolchain<zap::toolchains::gcc>(std::move(ti), exec);
        break;
        case toolchain_type::clang:
        case toolchain_type::apple_clang:
        tcp = new_toolchain<zap::toolchains::clang>(std::move(ti), exec);
        break;
        case toolchain_type::msvc:
        tcp = new_toolchain<zap::toolchains::msvc>(std::move(ti), exec);
        default:
        break;
    }
}

const toolchain&
get_toolchain()
{
    static tf::Executor exec;
    static toolchain_ptr tcp = nullptr;

    if (!tcp) {
        make_toolchain(tcp, exec);
    }

    return *tcp;
}

}
