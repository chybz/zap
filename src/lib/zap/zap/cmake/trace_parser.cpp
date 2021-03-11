#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <zap/cmake/trace_parser.hpp>
#include <zap/utils.hpp>
#include <zap/file_utils.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

using json = nlohmann::json;

bool
cmd::has(const std::string& key) const
{ return arg_keys.contains(key); }

trace_parser::trace_parser(const zap::toolchain& tc)
: tc_(tc),
hdr_re_(zap::re(zap::re_type::hdr)),
build_interface_{ "$<BUILD_INTERFACE:" }
{}

trace_parser::~trace_parser()
{}

void
trace_parser::parse(
    const std::string& src_dir,
    const std::string& trace_file
)
{
    source_dir_ = zap::fullpath(src_dir);

    set_project_dirs(source_dir_);

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
            parse_library_includes(src_dir, line);
        } else if (zap::contains(line, ts)) {
            parse_library_sources(line);
        }
    }
}

const project&
trace_parser::static_project() const
{ return static_; }

const project&
trace_parser::shared_project() const
{ return shared_; }

void
trace_parser::parse_subdirectory(const std::string& line)
{
    auto cmd = parse_cmd(line);

    std::string_view dirs = cmd.file;

    // Remove "source dir/" and trailing "/CMakeLists.txt"
    // "/CMakeLists.txt" is 15 chars
    dirs.remove_prefix(source_dir_.size() + 1);
    dirs.remove_suffix(15);

    if (dirs.empty()) {
        subdir_ = cmd.args[0];
    } else {
        subdir_ = zap::cat_dir(dirs, cmd.args[0]);
    }
}

void
trace_parser::parse_library(const std::string& line)
{
    auto cmd = parse_cmd(line);

    if (ignore_library(cmd) || cmd.args.empty()) {
        return;
    }

    const auto& lib = cmd.args[0];

    if (!cmd.has("ALIAS")) {
        // SHARED, STATIC, INTERFACE...
        die_unless(cmd.args.size() > 1, "invalid add_library");

        if (cmd.has("STATIC")) {
            static_.add_library(lib);
        } else if (cmd.has("SHARED")) {
            shared_.add_library(lib);
        } else {
            // INTERFACE or unspecified, valid for both schemes
            static_.add_library(lib);
            shared_.add_library(lib);
        }

        parse_library_sources(lib, cmd.args);
    } else {
        die_unless(cmd.args.size() == 3, "invalid add_library ALIAS");

        add_alias(lib, cmd.args[2]);
    }
}

void
trace_parser::parse_library_sources(const std::string& line)
{
    auto cmd = parse_cmd(line);

    parse_library_sources(cmd.args[0], cmd.args);
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

                if (fv.starts_with(source_dir_)) {
                    fv.remove_prefix(source_dir_.size() + 1);
                }

                headers.insert(zap::cat_file(subdir_, fv));
            }
        }
    }

    add_library_headers(lib, headers);
}

void
trace_parser::parse_library_includes(
    const std::string& src_dir,
    const std::string& line
)
{
    auto cmd = parse_cmd(line);

    for (const auto& a : cmd.args) {
        if (a.starts_with(build_interface_)) {
            auto dirs = parse_build_interface(a);
            auto inc_dir = zap::fullpath(dirs.front());

            if (inc_dir.starts_with(src_dir)) {
                set_library_interface(cmd.args[0], inc_dir);
            }
        }
    }
}

zap::string_views
trace_parser::parse_build_interface(const std::string& s) const
{
    std::string_view list = s;

    if (list.starts_with(build_interface_)) {
        list.remove_prefix(build_interface_.size());
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

    for (const auto& arg : l["args"]) {
        auto a = arg.get<std::string>();

        c.arg_keys.insert(a);
        c.args.emplace_back(std::move(a));
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

void
trace_parser::add_alias(const std::string& name, const std::string& target)
{
    static_.add_alias(name, target);
    shared_.add_alias(name, target);
}

void
trace_parser::add_library_headers(
    const std::string& name,
    const zap::string_set& headers
)
{
    static_.add_headers(name, headers);
    shared_.add_headers(name, headers);
}

void
trace_parser::set_library_interface(
    const std::string& name,
    const std::string& dir
)
{
    static_.set_interface_dir(name, dir);
    shared_.set_interface_dir(name, dir);
}

void
trace_parser::set_project_dirs(const std::string& dir)
{
    static_.clear();
    static_.dir = dir;

    shared_.clear();
    shared_.dir = dir;
}

}
