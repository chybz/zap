#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <zap/cmake/trace_parser.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

using json = nlohmann::json;

bool
cmd::has(const std::string& key) const
{ return arg_keys.contains(key); }

trace_parser::trace_parser(const zap::toolchain& tc)
: tc_(tc),
build_interface_{ "$<BUILD_INTERFACE:" }
{}

trace_parser::~trace_parser()
{}

project
trace_parser::parse(
    const std::string& src_dir,
    const std::string& trace_file
)
{
    set_project_dirs(zap::fullpath(src_dir));

    std::ifstream ifs(trace_file);

    auto al = make_cmd("add_library");
    auto tid = make_cmd("target_include_directories");

    for (std::string line; std::getline(ifs, line); ) {
        if (zap::contains(line, al)) {
            parse_library(p, line);
        } else if (zap::contains(line, tid)) {
            parse_library_includes(p, line);
        }
    }

    return p;
}

void
trace_parser::parse_library(project& p, const std::string& line)
{
    auto cmd = parse_cmd(line);

    if (ignore_library(cmd)) {
        return;
    }

    if (!cmd.has("ALIAS")) {
        // SHARED, STATIC, INTERFACE...
        die_unless(cmd.args.size() > 1, "invalid add_library");

        if (cmd.has("INTERFACE")) {
            static_.add_library(cmd.args[0]);
            shared_.add_library(cmd.args[0]);
        } else if (cmd.has("STATIC")) {

        }

        p.add_library(cmd.args[0]);
    } else {
        die_unless(cmd.args.size() == 3, "invalid add_library ALIAS");

        add_alias(cmd.args[0], cmd.args[2]);
    }
}

void
trace_parser::parse_library_includes(project& p, const std::string& line)
{
    auto cmd = parse_cmd(line);

    auto& l = p.get_library(cmd.args[0]);

    for (const auto& a : cmd.args) {
        if (a.starts_with(build_interface_)) {
            std::string_view dir = a;

            dir.remove_prefix(build_interface_.size());
            dir.remove_suffix(1);

            auto inc_dir = zap::fullpath(dir);

            if (inc_dir.starts_with(p.dir)) {
                l.interface_dir = inc_dir;
            }
        }
    }
}

bool
trace_parser::ignore_library(const cmd& c) const
{
    return
        cmd.has("IMPORTED")
        ||
        cmd.has("OBJECT")
        ||
        cmd.has("MODULE")
        ;
}

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
trace_parser::add_alias(const std::string& from, const std::string& to)
{
    add_alias(static_, from, to);
    add_alias(shared_, from, to);
}

void
trace_parser::add_alias(
    project& p,
    const std::string& from,
    const std::string& to
)
{
    if (!p.has_library(from)) {
        return;
    }

    p.add_library(from, to);
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
