#include <re2/re2.h>

#include <zap/cmake_configs.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>
#include <zap/executor.hpp>

namespace zap {

namespace detail {

const std::string config_suffixes[] = {
    "Config.cmake",
    "-config.cmake"
};

std::string
add_lib_pat()
{
    std::string pat = "(?i)";
    pat += "add_library\\s*\\(\\s*";
    pat += "(\\S+)";
    pat += "\\s+(?:SHARED|STATIC|INTERFACE)\\s+IMPORTED";
    pat += "\\s*\\)";

    return pat;
}

std::string
target_name_pat()
{
    std::string pat = "([\\w-]+)::([\\w-]+)";

    return pat;
}

std::string
inc_dirs_pat()
{
    std::string pat = "\\s*(\\S+)";
    pat += "\\.INTERFACE_INCLUDE_DIRECTORIES\\s*=\\s*";
    pat += "\"(.*)\"";

    return pat;
}

}

cmake_configs::cmake_configs(const zap::toolchain& tc, const std::string& root)
: package_configs(tc, root, package_config_type::cmake),
cmake_{ zap::find_cmd("cmake") },
add_lib_re_(detail::add_lib_pat()),
target_name_re_(detail::target_name_pat()),
inc_dirs_re_(detail::inc_dirs_pat())
{
    load_configs();
}

cmake_configs::~cmake_configs()
{}

bool
cmake_configs::has(const std::string& module) const
{ return names_.count(module) != 0; }

bool
cmake_configs::has_include_dirs(const std::string& module) const
{
    bool ret = false;
    auto it = inc_dirs_.find(module);

    if (it != inc_dirs_.end()) {
        ret = !it->second.empty();
    }

    return ret;
}

const inc_dir_set&
cmake_configs::include_dirs(const std::string& module) const
{ return inc_dirs_.at(module); }

void
cmake_configs::load_configs()
{
    cmake_.push_args({
        "-DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON"
    });

    auto cb = [&](auto& ctx, auto& path) {
        auto cmi = module_info(path);

        if (cmi.name.empty()) {
            return;
        }

        scan_config(ctx, cmi.name, cmi.config, true);

        auto tmp = "/tmp/zap";
        auto tdir = empty_temp_dir(tmp);

        for (const auto& conf : glob(path + "/*.cmake")) {
            if (conf == cmi.config) {
                continue;
            }

            scan_config(ctx, cmi.name, conf);
        }

        get_properties(ctx, cmi.name, tdir);

        rmpath(tdir);
    };

    zap::async_pool<decltype(cb), cmake_config_context> ap(tc().exec(), cb);

    for (const auto& dir : tc().make_arch_dirs(root(), "lib", "cmake")) {
        for (const auto& mdir : find_dirs(dir)) {
            auto adir = cat_dir(dir, mdir);
            ap.async(std::move(adir));
        }
    }

    auto& merged = ap.wait();

    process_modules(merged);

    names_ = std::move(merged.names);
    inc_dirs_ = std::move(merged.inc_dirs);
}

cmake_module_info
cmake_configs::module_info(const std::string& path) const
{
    cmake_module_info cmi;

    for (const auto& s : detail::config_suffixes) {
        auto configs = glob(path + "/*" + s);

        die_if(
            configs.size() > 1,
            "multiple CMake configs in ", path
        );

        if (configs.empty()) {
            continue;
        }

        cmi.config = std::move(configs.back());
        cmi.name = basename(cmi.config);
        cmi.name.erase(cmi.name.end() - s.size(), cmi.name.end());
        break;
    }

    return cmi;
}

void
cmake_configs::scan_config(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& f,
    bool main_config
)
{
    ctx.names.insert(module);

    auto data = slurp(f);

    if (main_config) {
        std::string find_comps_pat = "foreach\\(\\w+\\s+.+{?";
        find_comps_pat += join("_", module, "FIND_COMPONENTS");
        find_comps_pat += "}?\\)";

        re2::RE2 find_comps_re(find_comps_pat);
        re2::StringPiece input(data);

        if (re2::RE2::PartialMatch(input, find_comps_re)) {
            ctx.component_modules.insert(module);
        }
    }

    find_targets(ctx, module, data);
}

void
cmake_configs::find_targets(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& data
)
{
    re2::StringPiece input(data);
    re2::StringPiece match;
    re2::StringPiece mod;
    re2::StringPiece name;

    while (re2::RE2::FindAndConsume(&input, add_lib_re_, &match)) {
        ctx.target_names.insert({ match.data(), match.size() });
    }
}

void
cmake_configs::write_cmake_file(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& file
)
{
    mkfilepath(file);

    std::ofstream os(file);

    os
        << "cmake_minimum_required(VERSION 3.12)\n"
        << "project(detect_deps VERSION 1.0.0)\n"
        << "include(CMakePrintHelpers)\n"
        << "find_package(" << module << " CONFIG REQUIRED)\n"
        << "cmake_print_properties(\n"
        << "    TARGETS\n"
        ;

    for (const auto& t : ctx.target_names) {
        os << "        " << t << "\n";
    }

    os
        << "    PROPERTIES\n"
        << "        INTERFACE_INCLUDE_DIRECTORIES\n"
        << ")"
        << std::endl
        ;
}

void
cmake_configs::get_properties(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& dir
)
{
    if (ctx.target_names.empty()) {
        return;
    }

    auto cml = cat_file(dir, "CMakeLists.txt");

    write_cmake_file(ctx, module, cml);

    auto res = cmake_.run_silent_no_fail({ "-S", dir, "-B", dir });
    re2::StringPiece input(res.out);
    re2::StringPiece t;
    re2::StringPiece il;

    while (re2::RE2::FindAndConsume(&input, inc_dirs_re_, &t, &il)) {
        std::string target{ t.data(), t.size() };
        std::string_view list{ il.data(), il.size() };

        for (auto& dirv : split("\\s*;\\s*", list)) {
            if (!clean_dir(dirv)) {
                continue;
            }

            std::string cleaned(dirv.size() + 1, 0);
            cleaned.assign(dirv.begin(), dirv.end());
            cleaned.push_back('/');

            // TODO: per module inc dirs, may need per target
            ctx.inc_dirs[module].insert(std::move(cleaned));
        }
    }

    ctx.target_names.clear();
}

void
cmake_configs::process_modules(cmake_config_context& ctx)
{
    // for (auto& p : modules) {
    //     auto& m = p.second;

    //     strings moved;

    //     for (auto& tp : m.targets) {
    //         auto& t = tp.second;

    //         if (!t.module.empty() && t.module != p.first) {
    //             moved.push_back(tp.first);
    //             auto& parent = modules[t.module];
    //             parent.inc_dirs.merge(t.inc_dirs);
    //             t.inc_dirs.clear();
    //             parent.targets.insert(std::move(tp));
    //         } else {
    //             m.inc_dirs.merge(t.inc_dirs);
    //             t.inc_dirs.clear();
    //         }
    //     }
    // }

    // for (auto it = modules.begin(); it != modules.end(); ) {
    //     auto& m = it->second;

    //     if (m.inc_dirs.empty() || m.targets.empty()) {
    //         it = modules.erase(it);
    //     } else {
    //         ++it;
    //     }
    // }
}

bool
cmake_configs::clean_dir(std::string_view& dir) const
{
    if (dir.ends_with('/')) {
        dir.remove_suffix(1);
    }

    return !dir.empty();
}

}
