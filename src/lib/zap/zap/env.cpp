#include <filesystem>

#include <zap/env.hpp>
#include <zap/archiver.hpp>
#include <zap/fetchers/curl.hpp>
#include <zap/scope.hpp>
#include <zap/log.hpp>
#include <zap/utils.hpp>

namespace zap {

env::env(const std::string& root)
: root_(root),
executor_ptr_(std::make_unique<zap::executor>())
{
    namespace fs = std::filesystem;

    auto curp = root_.empty() ? fs::current_path() : fs::path(root_);
    auto buildp = curp / "build";

    cfg_.project_dir = curp.string();
    cfg_.build_dir = buildp.string();
    cfg_.archives_dir = (buildp / "archives").string();
    cfg_.work_dir = (buildp / "work").string();
    cfg_.local_prefix = "/local";
    cfg_.empty_dir = cat_dir(cfg_.home, "scan", "empty");
    cfg_.empty_source_file = cat_file(cfg_.home, "scan", "empty.cpp");

    mkpath(cfg_.empty_dir);
    touch_file(cfg_.empty_source_file);

    cfg_.package_file = (curp / "package.toml").string();
    cfg_.load_package_conf();

    toolchain_ptr_ = make_toolchain(cfg_, executor());

    make_fetcher();
}

env::~env()
{}

const std::string&
env::root() const
{ return root_; }

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

const config&
env::cfg() const
{ return cfg_; }

archive_info
env::download_archive(const std::string& url) const
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

void
env::make_fetcher()
{ fetcher_ptr_ = new_fetcher<zap::fetchers::curl>(cfg_); }

}
