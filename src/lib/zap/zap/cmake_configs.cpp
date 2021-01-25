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
location_pat()
{
    std::string pat = "\\s*(\\S+)";
    pat += "\\.LOCATION\\s*=\\s*";
    pat += "\"(.*)\"";

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

std::string
link_lib_pat()
{
    std::string pat = "\\s*(\\S+)";
    pat += "\\.INTERFACE_LINK_LIBRARIES\\s*=\\s*";
    pat += "\"(.*)\"";

    return pat;
}

}

///////////////////////////////////////////////////////////////////////////////
//
// Parallel parsing context
//
///////////////////////////////////////////////////////////////////////////////
void
cmake_config_context::merge(cmake_config_context& other)
{
    component_modules.merge(other.component_modules);
    names.merge(other.names);
    target_names.merge(other.target_names);
    targets.merge(other.targets);
    target_inc_dirs.merge(other.target_inc_dirs);
    libraries.merge(other.libraries);
    inc_dirs.merge(other.inc_dirs);

    for (auto& p : other.deps) {
        deps[p.first].merge(p.second);
    }

    for (auto& p : other.revdeps) {
        revdeps[p.first].merge(p.second);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
// CMake module extractor
//
///////////////////////////////////////////////////////////////////////////////
cmake_configs::cmake_configs(const zap::toolchain& tc, const std::string& root)
: package_configs(tc, root, package_config_type::cmake),
cmake_{ zap::find_cmd("cmake") },
add_lib_re_(detail::add_lib_pat()),
target_name_re_(detail::target_name_pat()),
location_re_(detail::location_pat()),
link_lib_re_(detail::link_lib_pat()),
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
cmake_configs::header_to_module(
    const std::string& name,
    const std::string& header,
    module_dep_info& module
) const
{
    std::cout << "WHOAA" << std::endl;
}

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
        std::string target{ match.data(), match.size() };

        ctx.target_names.insert(std::move(target));
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
        << "        INTERFACE_LINK_LIBRARIES\n"
        << "        LOCATION\n"
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

    parse_target_dirs(ctx, module, res.out);
    parse_target_location(ctx, module, res.out);
    parse_target_libs(ctx, module, res.out);

    ctx.target_names.clear();
}

void
cmake_configs::parse_target_dirs(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& data
)
{
    parse_target_list(
        data,
        inc_dirs_re_,
        [&](auto& target, auto& dirv) {
            if (!clean_dir(dirv)) {
                return;
            }

            std::string cleaned(dirv.size() + 1, 0);
            cleaned.assign(dirv.begin(), dirv.end());
            cleaned.push_back('/');

            ctx.target_inc_dirs[target].insert(cleaned);
            ctx.inc_dirs[module].insert(std::move(cleaned));
        }
    );

    for (const auto& p : ctx.target_inc_dirs) {
        ctx.targets[module].insert(p.first);
    }
}

void
cmake_configs::parse_target_location(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& data
)
{
    parse_target_var(
        data,
        location_re_,
        [&](auto& target, auto& location) {
            ctx.libraries.try_emplace(std::move(target), std::move(location));
        }
    );
}

void
cmake_configs::parse_target_libs(
    cmake_config_context& ctx,
    const std::string& module,
    const std::string& data
)
{
    parse_target_list(
        data,
        link_lib_re_,
        [&](auto& target, auto& libv) {
            std::string lib{ libv.data(), libv.size() };

            ctx.deps[target].insert(lib);
            ctx.revdeps[lib].insert(target);
        }
    );
}

template <typename Callable>
void
cmake_configs::parse_target_var(
    const std::string& data,
    const re2::RE2& re,
    Callable&& cb
)
{
    re2::StringPiece input(data);
    re2::StringPiece t;
    re2::StringPiece v;

    while (re2::RE2::FindAndConsume(&input, re, &t, &v)) {
        std::string target{ t.data(), t.size() };
        std::string_view var{ v.data(), v.size() };

        if (var == "<NOTFOUND>") {
            continue;
        }

        cb(target, var);
    }
}

template <typename Callable>
void
cmake_configs::parse_target_list(
    const std::string& data,
    const re2::RE2& re,
    Callable&& cb
)
{
    parse_target_var(
        data,
        re,
        [&](auto& target, auto& list) {
            for (auto& item : split("\\s*;\\s*", list)) {
                cb(target, item);
            }
        }
    );
}

void
cmake_configs::process_modules(cmake_config_context& ctx)
{
    string_set_map dir_groups;

    for (const auto& p : ctx.target_inc_dirs) {
        for (const auto& dir : p.second) {
            dir_groups[dir].insert(p.first);
        }
    }

    // Collect uniquely identifiable targets: those with a unique include
    // directory
    for (auto it = dir_groups.begin(); it != dir_groups.end(); ) {
        const auto& dir = it->first;
        const auto& targets = it->second;

        if (targets.size() == 1) {
            const auto& target = *targets.begin();
            dir_to_target_.try_emplace(dir, target);
            unique_targets_.insert(target);
            it = dir_groups.erase(it);
        } else {
            ++it;
        }
    }

    // Erase targets that are now unique
    for (auto it = dir_groups.begin(); it != dir_groups.end(); ) {
        auto& dir = it->first;
        auto& targets = it->second;

        for (auto tit = targets.begin(); tit != targets.end(); ) {
            if (unique_targets_.contains(*tit)) {
                tit = targets.erase(tit);
            } else {
                ++tit;
            }
        }

        if (targets.empty()) {
            it = dir_groups.erase(it);
        } else {
            ++it;
        }
    }

    std::cout << "modules processed" << std::endl;
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
