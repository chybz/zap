#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <zap/cmake/trace_parser.hpp>
#include <zap/utils.hpp>
#include <zap/file_utils.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
//
// Utilities
//
///////////////////////////////////////////////////////////////////////////////
namespace detail {

template <typename T>
auto extract(const T& v)
{ return v; }

template <typename T, typename U>
auto extract(const std::pair<T, U>& p)
{ return p.first; }

}

///////////////////////////////////////////////////////////////////////////////
//
// cmd
//
///////////////////////////////////////////////////////////////////////////////
bool
cmd::has(const std::string& key) const
{ return arg_keys.contains(key); }

///////////////////////////////////////////////////////////////////////////////
//
// Trace Parser
//
///////////////////////////////////////////////////////////////////////////////
trace_parser::trace_parser(const zap::toolchain& tc)
: tc_(tc),
hdr_re_(zap::re(zap::re_type::hdr)),
build_interface_{ "$<BUILD_INTERFACE:" },
install_interface_{ "$<INSTALL_INTERFACE:" }
{}

trace_parser::~trace_parser()
{}

void
trace_parser::parse(
    const std::string& src_dir,
    const std::string& trace_file
)
{
    src_dir_ = zap::fullpath(src_dir);

    p_.clear();
    p_.dir = src_dir_;

    std::ifstream ifs(trace_file);

    auto asd = make_cmd("add_subdirectory");
    auto al = make_cmd("add_library");
    auto tid = make_cmd("target_include_directories");
    auto tll = make_cmd("target_link_libraries");
    auto ts = make_cmd("target_sources");

    for (std::string line; std::getline(ifs, line); ) {
        if (zap::contains(line, asd)) {
            parse_subdirectory(line);
        } else if (zap::contains(line, al)) {
            parse_library(line);
        } else if (zap::contains(line, tid)) {
            parse_library_includes(line);
        } else if (zap::contains(line, tll)) {
            parse_library_deps(line);
        } else if (zap::contains(line, ts)) {
            parse_library_sources(line);
        }
    }

    handle_deps();
}

void
trace_parser::post_install(const std::string& inst_dir)
{
    inst_dir_ = zap::fullpath(inst_dir);

    p_.clean_libraries(inst_dir_);
}

void
trace_parser::handle_deps()
{
    auto external_lib = [&](const auto& item) {
        return !seen_libs_.contains(detail::extract(item));
    };

    for (auto& p : deps_) {
        std::erase_if(p.second, external_lib);
    }

    std::erase_if(rev_deps_, external_lib);
}

const zap::cmake::project&
trace_parser::project() const
{ return p_; }

void
trace_parser::parse_subdirectory(const std::string& line)
{
    auto cmd = parse_cmd(line);

    std::string_view dirs = cmd.file;

    // Remove "source dir/" and trailing "/CMakeLists.txt"
    // "/CMakeLists.txt" is 15 chars
    dirs.remove_prefix(src_dir_.size() + 1);
    dirs.remove_suffix(15);

    if (dirs.empty()) {
        subdir_ = cmd.subject;
    } else {
        subdir_ = zap::cat_dir(dirs, cmd.subject);
    }
}

void
trace_parser::parse_library(const std::string& line)
{
    auto cmd = parse_cmd(line);

    // TODO: handle OBJECT case and scan for headers too
    if (ignore_library(cmd) || cmd.args.empty()) {
        return;
    }

    const auto& lib = cmd.subject;

    seen_libs_.insert(lib);

    if (cmd.has("ALIAS")) {
        die_unless(cmd.args.size() == 2, "invalid add_library ALIAS");

        p_.add_alias(lib, cmd.args[1]);
    } else {
        // SHARED, STATIC, INTERFACE...
        die_unless(cmd.args.size() > 0, "invalid add_library");

        p_.add_library(lib);

        parse_library_sources(lib, cmd.args);
    }
}

void
trace_parser::parse_library_sources(const std::string& line)
{
    auto cmd = parse_cmd(line);

    parse_library_sources(cmd.subject, cmd.args);
}

void
trace_parser::parse_library_sources(
    const std::string& lib,
    const zap::strings& args
)
{
    zap::string_set headers;

    for (const auto& a : args) {
        for (const auto& file : parse_build_interface(a)) {
            if (re2::RE2::FullMatch(file, hdr_re_)) {
                std::string_view fv = file;

                if (fv.starts_with(src_dir_)) {
                    fv.remove_prefix(src_dir_.size() + 1);
                }

                headers.insert(zap::cat_file(subdir_, fv));
            }
        }
    }

    p_.add_headers(lib, headers);
}

void
trace_parser::parse_library_includes(const std::string& line)
{
    auto cmd = parse_cmd(line);
    std::string siface;
    std::string diface = "include";

    for (const auto& a : cmd.args) {
        if (a.starts_with(build_interface_)) {
            auto dirs = parse_build_interface(a);
            auto inc_dir = zap::fullpath(dirs.front());

            if (inc_dir.starts_with(src_dir_)) {
                siface = inc_dir;

                zap::files files;

                zap::add_files(files, inc_dir, zap::re(zap::re_type::hdr));
                p_.add_headers(cmd.subject, files);
            }
        } else if (a.starts_with(install_interface_)) {
            auto dirs = parse_install_interface(a);

            if (!dirs.empty()) {
                diface = dirs.front();
            }
        }
    }

    p_.set_interface_dirs(cmd.subject, siface, diface);
}

void
trace_parser::parse_library_deps(const std::string& line)
{
    auto cmd = parse_cmd(line);
    auto& lib_deps = deps_[cmd.subject];

    for (const auto& a : cmd.args) {
        if (not_a_library(a)) {
            continue;
        }

        // Accumulate possible dependencies for now as they might be
        // declared out of order. We'll consolidate after the entire
        // trace is processed
        for (const auto& depv : zap::split(";", a)) {
            std::string dep(depv.data(), depv.size());

            rev_deps_[dep].insert(cmd.subject);
            lib_deps.insert(std::move(dep));
        }
    }
}

bool
trace_parser::not_a_library(const std::string& s) const
{
    return
        s.empty()
        ||
        // I saw compiler/linker flags...
        s.starts_with('-')
        ||
        // Generator expressions
        s.starts_with('$')
        ||
        // Ignore keywords
        s == "PUBLIC" || s == "PRIVATE" || s == "INTERFACE"
        ;
}

zap::string_views
trace_parser::parse_build_interface(const std::string& s) const
{ return parse_interface(s, build_interface_); }

zap::string_views
trace_parser::parse_install_interface(const std::string& s) const
{ return parse_interface(s, install_interface_); }

zap::string_views
trace_parser::parse_interface(
    const std::string& s,
    const std::string_view& interface
) const
{
    std::string_view list = s;

    if (list.starts_with(interface)) {
        list.remove_prefix(interface.size());
        list.remove_suffix(1);
    }

    return zap::split(";", list);
}

bool
trace_parser::ignore_library(const cmd& c) const
{ return c.has("IMPORTED") || c.has("OBJECT") || c.has("MODULE"); }

cmd
trace_parser::parse_cmd(const std::string& line) const
{
    cmd c;

    auto l = json::parse(line);
    bool first = true;

    for (const auto& arg : l["args"]) {
        auto a = arg.get<std::string>();

        if (first) {
            c.subject = std::move(a);
            first = false;
        } else {
            c.arg_keys.insert(a);
            c.args.emplace_back(std::move(a));
        }
    }

    c.file = l["file"].get<std::string>();
    c.frame = l["frame"].get<std::size_t>();

    return c;
}

std::string
trace_parser::make_cmd(const std::string& cmd) const
{
    std::ostringstream oss;

    oss <<  "\"cmd\":" << '"' << cmd << '"';

    return oss.str();
}

}
