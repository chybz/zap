#include <filesystem>

#include <zap/env.hpp>
#include <zap/archiver.hpp>
#include <zap/scope.hpp>
#include <zap/log.hpp>
#include <zap/utils.hpp>

namespace zap {

env::env(const env_opts& opts)
: opts_(opts),
sys_db_ptr_(new_sys_db())
{
    if (opts_.name.empty() && !opts_.no_init) {
        die_unless(
            sys_db().has_default_env(),
            "no default environment"
        );

        auto de = sys_db().get_env(sys_db().default_env());

        opts_.name = de.name;
        root_ = de.root;
    }

    if (!opts_.no_init) {
        init();
    }
}

env::~env()
{}

zap::sys_db&
env::sys_db() const
{ return *sys_db_ptr_; }

zap::env_db&
env::env_db() const
{ return *env_db_ptr_; }

const std::string&
env::root() const
{ return root_; }

const std::string&
env::operator[](const std::string& name) const
{ return paths_[name]; }

const string_map&
env::build_env() const
{ return build_env_; }

zap::executor&
env::executor() const
{ return *executor_ptr_; }

const zap::os_info&
env::os_info() const
{ return os_info_; }

const zap::toolchain&
env::toolchain() const
{ return *toolchain_ptr_; }

const zap::fetcher&
env::fetcher() const
{ return *fetcher_ptr_; }

archive_info
env::download_archive(const std::string& url) const
{
    scope s;
    archive_info ai{url};
    env_db_archive ar;
    bool download_needed = true;

    const auto& archives_dir = paths_["archives"];

    mkpath(archives_dir);

    if (env_db().has_archive(url, ar)) {
        ai.file = cat_file(archives_dir, ar.file);

        download_needed = !file_exists(ai.file);
    }

    if (download_needed) {
        download_archive(s, ai);
    }

    extract_archive(s, ai);

    return ai;
}

void
env::download_archive(scope& s, archive_info& ai) const
{
    const auto& archives_dir = paths_["archives"];

    set_temp_dir(s, ai);

    fetcher().download(ai.url, ai.temp_dir);

    auto [ dlok, file ] = unique_file(ai.temp_dir);

    die_unless(
        dlok,
        "unable to find dowloaded file in: ", ai.temp_dir
    );

    ai.file = cat_file(archives_dir, file);

    rename(cat_file(ai.temp_dir, file), ai.file);

    env_db().add_archive(env_db_archive{ ai.url, file });
}

void
env::extract_archive(scope& s, archive_info& ai) const
{
    set_temp_dir(s, ai);

    archiver ar(paths_, ai.file);

    ar.verify();
    ar.extract(ai.temp_dir);

    auto [ exok, dir ] = unique_dir(ai.temp_dir);

    die_unless(
        exok,
        "archive extracts into multiple directories: ", ai.url
    );

    auto pos = dir.rfind('-');

    die_if(
        pos == std::string::npos,
        "unable to extract name and version from: ", dir
    );

    ai.dir = cat_dir(paths_["work"], dir);

    if (directory_exists(ai.dir)) {
        rmpath(ai.dir);
    }

    mkpath(ai.dir);

    ai.name = dir.substr(0, pos);
    ai.version = dir.substr(pos + 1);
    ai.source_dir = cat_dir(ai.dir, "src");

    rename(cat_dir(ai.temp_dir, dir), ai.source_dir);
}

void
env::set_temp_dir(scope& s, archive_info& ai) const
{
    if (!ai.temp_dir.empty()) {
        return;
    }

    ai.temp_dir = empty_temp_dir(paths_["archives"]);

    s.push_rmpath(ai.temp_dir);
}

void
env::init()
{
    namespace fs = std::filesystem;

    auto rootp = fs::path(root_);
    auto buildp = rootp / "build";

    paths_.sm.emplace("root", rootp.string());
    paths_.sm.emplace("etc", (rootp / "etc").string());
    paths_.sm.emplace("include", (rootp / "include").string());
    paths_.sm.emplace("lib", (rootp / "lib").string());
    paths_.sm.emplace("pkgconfig", (rootp / "lib" / "pkgconfig").string());
    paths_.sm.emplace("cmake", (rootp / "lib" / "cmake").string());
    paths_.sm.emplace("build", buildp.string());
    paths_.sm.emplace("archives", (buildp / "archives").string());
    paths_.sm.emplace("work", (buildp / "work").string());
    paths_.sm.emplace("tmp", (buildp / "tmp").string());

    env_db_ptr_ = new_env_db(paths_["root"]);
    executor_ptr_ = std::make_unique<zap::executor>();
    toolchain_ptr_ = make_toolchain(paths_, executor());

    build_env_.emplace("CC", toolchain().cc_cmd());
    build_env_.emplace("CXX", toolchain().cxx_cmd());
    build_env_.emplace("CPATH", paths_["include"]);
    build_env_.emplace("LIBRARY_PATH", paths_["lib"]);
    build_env_.emplace("PKG_CONFIG_PATH", paths_["pkgconfig"]);

    make_fetcher();
}

void
env::make_fetcher()
{ fetcher_ptr_ = new_fetcher<fetcher>(paths_); }

}
