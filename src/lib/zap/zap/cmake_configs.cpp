#include <re2/re2.h>

#include <zap/cmake_configs.hpp>
#include <zap/utils.hpp>
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

struct cmake_context
{
    cmake_modules modules;

    bool has_targets(const std::string& module) const
    {
        if (!modules.contains(module)) {
            return false;
        }

        return !modules.at(module).targets.empty();
    }

    bool has_inc_dirs(const std::string& module) const
    {
        if (!modules.contains(module)) {
            return false;
        }

        return !modules.at(module).inc_dirs.empty();
    }

    void erase(const std::string& module)
    { modules.erase(module); }

    void merge(cmake_context& other)
    { modules.merge(other.modules); }
};

}

cmake_configs::cmake_configs(const zap::toolchain& tc, const std::string& root)
: tc_{ tc },
root_(root),
cmake_{ zap::find_cmd("cmake") },
add_lib_re_(detail::add_lib_pat()),
target_name_re_(detail::target_name_pat()),
inc_dirs_re_(detail::inc_dirs_pat())
{
    load_configs();
}

cmake_configs::~cmake_configs()
{}

const cmake_modules&
cmake_configs::modules() const
{ return modules_; }

bool
cmake_configs::has(const std::string& module) const
{ return modules_.count(module) != 0; }

bool
cmake_configs::has_include_dirs(const std::string& module) const
{
    bool ret = false;
    auto it = modules_.find(module);

    if (it != modules_.end()) {
        ret = !it->second.inc_dirs.empty();
    }

    return ret;
}

const cmake_configs::inc_dir_set&
cmake_configs::include_dirs(const std::string& module) const
{ return modules_.at(module).inc_dirs; }

void
cmake_configs::load_configs()
{
    // I promise I tried *very* hard to have this right, without too much
    // kludge.
    // There are just too many broken .cmake files not respecting variables and
    // other niceties
    cmake_.push_args({
        "-DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON"
    });

    auto cb = [&](auto& ctx, auto& path) {
        auto cmi = module_info(path);

        if (cmi.name.empty()) {
            return;
        }

        scan_config(ctx.modules, cmi.name, cmi.config);

        auto tmp = "/tmp/zap";
        auto tdir = empty_temp_dir(tmp);

        for (const auto& conf : glob(path + "/*.cmake")) {
            if (conf == cmi.config) {
                continue;
            }

            scan_config(ctx.modules, cmi.name, conf);
        }

        get_properties(ctx.modules, cmi.name, tdir);

        rmpath(tdir);
    };

    zap::async_pool<decltype(cb), detail::cmake_context> ap(tc_.exec(), cb);

    for (const auto& dir : tc_.make_arch_dirs(root_, "lib", "cmake")) {
        for (const auto& mdir : find_dirs(dir)) {
            auto adir = cat_dir(dir, mdir);
            ap.async(std::move(adir));
        }
    }

    auto& merged = ap.wait();

    process_modules(merged.modules);
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

bool
cmake_configs::is_module_config(const std::string& path) const
{
    for (const auto& s : detail::config_suffixes) {
        if (path.ends_with(s)) {
            return true;
        }
    }

    return false;
}

void
cmake_configs::scan_config(
    cmake_modules& modules,
    const std::string& module,
    const std::string& f
)
{
    auto data = slurp(f);

    if (!modules.contains(module)) {
        auto find_comps = join("_", module, "FIND_COMPONENTS");

        cmake_module mod{
            .name = module,
            .has_components = data.find(find_comps) != std::string::npos
        };

        modules.try_emplace(module, std::move(mod));
    }

    find_targets(modules, module, data);
}

void
cmake_configs::find_targets(
    cmake_modules& modules,
    const std::string& module,
    const std::string& data
)
{
    re2::StringPiece input(data);
    re2::StringPiece match;
    re2::StringPiece mod;
    re2::StringPiece name;

    while (re2::RE2::FindAndConsume(&input, add_lib_re_, &match)) {
        cmake_target t;

        if (re2::RE2::FullMatch(match, target_name_re_, &mod, &name)) {
            // Target with namespace
            t.module.assign(mod.begin(), mod.end());
            t.name.assign(name.begin(), name.end());
        } else {
            t.name.assign(match.begin(), match.end());
        }

        auto tname = match.as_string();

        modules[module].targets.try_emplace(std::move(tname), std::move(t));
    }
}

void
cmake_configs::write_cmake_file(
    const cmake_modules& modules,
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

    for (const auto& p : modules.at(module).targets) {
        os << "        " << p.first << "\n";
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
    cmake_modules& modules,
    const std::string& module,
    const std::string& dir
)
{
    if (modules[module].targets.empty()) {
        return;
    }

    auto cml = cat_file(dir, "CMakeLists.txt");

    write_cmake_file(modules, module, cml);

    auto res = cmake_.run_silent_no_fail({ "-S", dir, "-B", dir });
    re2::StringPiece input(res.out);
    re2::StringPiece t;
    re2::StringPiece il;

    auto& mod = modules[module];
    string_view_set seen_targets;

    while (re2::RE2::FindAndConsume(&input, inc_dirs_re_, &t, &il)) {
        std::string_view list{ il.data(), il.size() };

        seen_targets.insert({ t.data(), t.size() });

        for (const auto& dirv : split("\\s*;\\s*", list)) {
            std::string dir{ dirv.data(), dirv.size() };

            mod.inc_dirs.insert(std::move(dir));
        }
    }

    // Remove targets without include dirs
    strings to_remove;

    for (const auto& p : mod.targets) {
        if (!seen_targets.contains(p.first)) {
            to_remove.push_back(p.first);
        }
    }

    for (const auto& : to_remove) {
        mod.targets.erase()
    }
}

void
cmake_configs::process_modules(cmake_modules& modules)
{
    strings to_clean;

    for (auto& p : modules) {
        auto& m = p.second;

        strings moved;

        for (auto& tp : m.targets) {
            auto& t = tp.second;

            if (!t.module.empty() && t.module != p.first) {
                moved.push_back(tp.first);
                auto& parent = modules[t.module];
                parent.targets.insert(std::move(tp));
            }
        }

        if (!moved.empty() && moved.size() == m.targets.size()) {
            to_clean.push_back(p.first);
        }
    }

    for (const auto& m : to_clean) {
        modules.erase(m);
    }
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
