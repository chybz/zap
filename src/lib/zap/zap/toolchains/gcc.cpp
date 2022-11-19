#include <unordered_set>

#include <zap/toolchains/gcc.hpp>
#include <zap/file_utils.hpp>
#include <zap/utils.hpp>
#include <zap/scope.hpp>
#include <zap/scan_context.hpp>

namespace zap::toolchains {

///////////////////////////////////////////////////////////////////////////////
//
// GCC toolchain
//
///////////////////////////////////////////////////////////////////////////////
gcc::gcc(
    const zap::env_paths& ep,
    zap::toolchain_info&& ti,
    zap::executor& exec
)
: zap::toolchain(ep, std::forward<zap::toolchain_info>(ti), exec),
extract_line_re_("(?:ZAP_SOURCE:)?\\s+(.*)\\s+\\\\?\n")
{
    set_target_arch(cxx().get_line({ .args = { "-dumpmachine" } }));

    scanner() = cxx();

    scanner().push_args({
        "-x", "c++",
        "-std=c++20", // TOFIX: make this somewhat configurable
        "-M", "-MG", "-MT", "ZAP_SOURCE",
        zap::cat("-I", empty_dir()),
        "-nostdinc", "-nostdinc++"
    });

    nm().push_args({ "-u", "-g" });

    zap::try_find_prog("ccache", info_.compiler_launcher);

    find_std_headers();
}

gcc::~gcc()
{}

void
gcc::scan_files(
    const zap::strings& inc_dirs,
    const std::string& dir,
    const zap::files& f,
    zap::strings& deps
) const
{
    if (f.empty()) {
        return;
    }

    zap::strings slash_inc_dirs;
    auto sc = scanner();

    for (const auto& inc_dir : inc_dirs) {
        sc.push_args({ zap::cat("-I", inc_dir) });
        slash_inc_dirs.emplace_back(re2::RE2::QuoteMeta(inc_dir + '/'));
    }

    slash_inc_dirs.emplace_back(re2::RE2::QuoteMeta(dir + '/'));

    auto header_pat = "(?:" + zap::join("|", slash_inc_dirs) + ")?(\\S+)";
    re2::RE2 header_re(header_pat);

    auto cb = [&](auto& ctx, const auto& dir, const auto& file) {
        auto res = sc.run_silent_no_fail(
            { .args = { zap::cat_file(dir, file) } }
        );

        extract_deps(header_re, res, ctx.deps);
    };

    zap::async_pool<decltype(cb), zap::scan_context> ap(executor(), cb);

    for (const auto& file : f) {
        ap.async(dir, file);
    }

    auto& merged = ap.wait();

    for (auto&& d : merged.deps) {
        deps.emplace_back(std::move(d));
    }
}

zap::strings
gcc::local_lib_deps(
    const std::string& file,
    const zap::string_set& accepted
) const
{
    zap::strings deps;

    if (zap::lib_is_shared(file)) {
        local_shared_lib_deps(file, accepted, deps);
    } else if (zap::lib_is_static(file)) {
        local_static_lib_deps(file, accepted, deps);
    }

    return deps;
}

zap::strings
gcc::local_lib_deps(
    const std::string& file,
    const zap::string_map& accepted
) const
{
    zap::strings deps;

    if (zap::lib_is_shared(file)) {
        local_shared_lib_deps(file, accepted, deps);
    } else if (zap::lib_is_static(file)) {
        local_static_lib_deps(file, accepted, deps);
    }

    return deps;
}

void
gcc::configure_std_header_finder(zap::prog& finder) const
{}

void
gcc::find_std_headers()
{
    std::string filter{ " (.*(?:c\\+\\+|" + name() + ").*)" };
    auto cxx_finder = cxx();

    cxx_finder.push_args({
        "-x", "c++",
        "-Wp,-v",
        "-fsyntax-only",
        empty_file()
    });

    configure_std_header_finder(cxx_finder);

    auto res = cxx_finder.run_silent();
    auto hdirs = zap::split_and_map_lines(filter, res.err);

    find_std_headers(std_headers_, hdirs);
}

void
gcc::find_std_headers(zap::files& stdh, const string_views& hdirs) const
{
    string_view_set seen;

    for (const auto& hdir : hdirs) {
        if (seen.count(hdir) > 0) {
            continue;
        }

        auto files = find_files(hdir);

        stdh.insert(
            std::make_move_iterator(files.begin()),
            std::make_move_iterator(files.end())
        );

        seen.insert(hdir);
    }
}

void
gcc::extract_deps(
    const re2::RE2& re,
    zap::prog_result& res,
    zap::string_set& deps
) const
{
    if (res.out.empty()) {
        return;
    }

    re2::StringPiece input(res.out);
    re2::StringPiece match;
    re2::StringPiece hmatch;

    while (re2::RE2::Consume(&input, extract_line_re_, &match)) {
        while (re2::RE2::FindAndConsume(&match, re, &hmatch)) {
            std::string h(hmatch.begin(), hmatch.end());

            deps.insert(std::move(h));
        }
    }
}

template <typename Associative>
void
gcc::local_shared_lib_deps(
    const std::string& file,
    const Associative& accepted,
    zap::strings& deps
) const
{
    auto re = zap::compose_re(
        "\\s+", zap::re_type::lib_link_name, " => .*"
    );

    auto res = ldd().run_silent({ .args = { file } });

    for (auto&& l : zap::split_and_map_lines(re, res.out)) {
        std::string sl{l};

        if (accepted.count(sl) != 0) {
            deps.emplace_back(std::move(sl));
        }
    }
}

template <typename Associative>
void
gcc::local_static_lib_deps(
    const std::string& file,
    const Associative& accepted,
    zap::strings& deps
) const
{}

}
