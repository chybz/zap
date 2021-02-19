#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include <zap/cmake/trace_parser.hpp>
#include <zap/utils.hpp>
#include <zap/log.hpp>

namespace zap::cmake {

using json = nlohmann::json;

trace_parser::trace_parser()
: build_interface_{ "$<BUILD_INTERFACE:" }
{}

trace_parser::~trace_parser()
{}

project
trace_parser::parse(
    const std::string& src_dir,
    const std::string& trace_file
)
{
    project p{ zap::fullpath(src_dir) };
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

    if (ignore_library(cmd.arg_keys)) {
        return;
    }

    if (cmd.arg_keys.contains("ALIAS")) {
        die_unless(cmd.args.size() == 3, "invalid add_library ALIAS");

        const auto& name = cmd.args[0];
        const auto& target = cmd.args[2];

        if (p.libs.contains(name)) {
            p.libs[name].alias = target;
            p.aliases[target] = name;
        } else if (p.libs.contains(target)) {
            p.libs[target].alias = name;
            p.aliases[name] = target;
        }
    } else {
        // SHARED, STATIC, INTERFACE...
        die_unless(cmd.args.size() > 1, "invalid add_library");

        p.libs[cmd.args[0]].name = cmd.args[0];
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
                for (auto& header : zap::find_files(inc_dir)) {
                    l.headers.insert(std::move(header));
                }
            }
        }
    }
}

bool
trace_parser::ignore_library(const zap::string_set& arg_keys) const
{
    return
        arg_keys.contains("IMPORTED")
        ||
        arg_keys.contains("OBJECT")
        ||
        arg_keys.contains("MODULE")
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

}
